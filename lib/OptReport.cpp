#include "OptDebugger/OptReport.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace optdbg {

namespace {

// maps abstract severity levels to specific terminal ansi color codes
llvm::raw_ostream::Colors colorForSeverity(SeverityLevel S) {
  switch (S) {
  case SeverityLevel::Critical: return llvm::raw_ostream::RED;
  case SeverityLevel::High:     return llvm::raw_ostream::YELLOW;
  case SeverityLevel::Medium:   return llvm::raw_ostream::MAGENTA;
  case SeverityLevel::Low:      return llvm::raw_ostream::CYAN;
  case SeverityLevel::Info:     return llvm::raw_ostream::WHITE;
  }
  return llvm::raw_ostream::WHITE;
}

}

TerminalReporter::TerminalReporter(llvm::raw_ostream &OS, ReportConfig Config)
    : OS(OS), Cfg(std::move(Config)) {}

// formats and prints a horizontal separator line for terminal output
void TerminalReporter::printSeparator(char Ch, unsigned Width) {
  for (unsigned I = 0; I < Width; ++I)
    OS << Ch;
  OS << "\n";
}

// prints a text string with specified ansi colors, respecting the user's color configuration
void TerminalReporter::printColoredLine(llvm::StringRef Text,
                                         llvm::raw_ostream::Colors Color) {
  if (Cfg.UseColor)
    OS.changeColor(Color, true);
  OS << Text;
  if (Cfg.UseColor)
    OS.resetColor();
  OS << "\n";
}

// renders the main header and pipeline execution summary to the terminal
void TerminalReporter::printHeader(const AnalysisSession &Session) {
  printSeparator('=');
  printColoredLine("  LLVM Optimization Failure Debugger",
                   llvm::raw_ostream::CYAN);
  printColoredLine("  Why wasn't my code optimized?",
                   llvm::raw_ostream::WHITE);
  printSeparator('=');

  OS << "  Pipeline : " << Session.PassPipelineUsed << "\n";
  OS << "  Remarks  : " << Session.Remarks.size() << " total\n";

  size_t Missed = 0, Applied = 0;
  for (const Remark &R : Session.Remarks) {
    if (R.isMissed()) ++Missed;
    else if (R.isApplied()) ++Applied;
  }
  OS << "  Missed   : " << Missed << "\n";
  OS << "  Applied  : " << Applied << "\n";
  OS << "\n";
}

// calculates and prints aggregate ir metrics, highlighting overall changes and severity distributions
void TerminalReporter::printSummaryStats(const AnalysisSession &Session) {
  const ModuleDiff &D = Session.Diff;

  if (!D.hasChanges() && Session.Diagnostics.empty()) {
    printColoredLine("  No optimization opportunities detected.",
                     llvm::raw_ostream::GREEN);
    OS << "\n";
    return;
  }

  printSeparator('-');
  printColoredLine("  IR Statistics", llvm::raw_ostream::CYAN);
  printSeparator('-');

  OS << "  Functions  : before=" << (D.ModifiedFunctions + D.UnchangedFunctions +
                                      D.RemovedFunctions)
     << "  after=" << (D.ModifiedFunctions + D.UnchangedFunctions +
                       D.AddedFunctions) << "\n";
  OS << "  Modified   : " << D.ModifiedFunctions << "\n";
  OS << "  Inlined    : " << D.RemovedFunctions  << "\n";
  OS << "  Instructions before : " << D.TotalBeforeInstructions << "\n";
  OS << "  Instructions after  : " << D.TotalAfterInstructions  << "\n";

  int64_t Delta = D.instructionDelta();
  OS << "  Instruction delta   : ";
  if (Delta < 0) {
    if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::GREEN);
    OS << Delta << " (reduced)";
  } else if (Delta > 0) {
    if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::YELLOW);
    OS << "+" << Delta << " (increased — possible inlining expansion)";
  } else {
    OS << "0 (no change)";
  }
  if (Cfg.UseColor) OS.resetColor();
  OS << "\n\n";

  if (!Session.Diagnostics.empty()) {
    unsigned Critical = 0, High = 0, Medium = 0, Low = 0;
    for (const DiagnosticResult &DR : Session.Diagnostics) {
      switch (DR.Severity) {
      case SeverityLevel::Critical: ++Critical; break;
      case SeverityLevel::High:     ++High;     break;
      case SeverityLevel::Medium:   ++Medium;   break;
      case SeverityLevel::Low:      ++Low;      break;
      case SeverityLevel::Info:                 break;
      }
    }

    printSeparator('-');
    printColoredLine("  Missed Optimization Summary", llvm::raw_ostream::CYAN);
    printSeparator('-');

    if (Critical > 0) {
      if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::RED, true);
      OS << "  [!!] CRITICAL : " << Critical;
      if (Cfg.UseColor) OS.resetColor();
      OS << "\n";
    }
    if (High > 0) {
      if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::YELLOW, true);
      OS << "  [! ] HIGH     : " << High;
      if (Cfg.UseColor) OS.resetColor();
      OS << "\n";
    }
    if (Medium > 0) {
      if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::MAGENTA);
      OS << "  [~ ] MEDIUM   : " << Medium;
      if (Cfg.UseColor) OS.resetColor();
      OS << "\n";
    }
    if (Low > 0) {
      if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::CYAN);
      OS << "  [-  ] LOW     : " << Low;
      if (Cfg.UseColor) OS.resetColor();
      OS << "\n";
    }
    OS << "\n";
  }
}

// formats the localized context and severity header for a single compiler diagnostic
void TerminalReporter::printDiagnosticHeader(const DiagnosticResult &D) {
  printSeparator('=');
  if (Cfg.UseColor)
    OS.changeColor(colorForSeverity(D.Severity), true);

  OS << severityToEmoji(D.Severity) << " [" << severityToString(D.Severity)
     << "] " << D.ShortReason << "\n";

  if (Cfg.UseColor) OS.resetColor();

  OS << "  Pass     : " << D.PassName << "\n";
  OS << "  Function : @" << D.FunctionName << "\n";

  if (D.Location.isValid())
    OS << "  Location : " << D.Location.format() << "\n";

  if (D.EstimatedSpeedup > 0.0) {
    OS << "  Potential speedup if fixed: ";
    if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::GREEN);
    OS << llvm::format("%.1fx", D.EstimatedSpeedup);
    if (Cfg.UseColor) OS.resetColor();
    OS << "\n";
  }
}

// outputs detailed multi-line reasoning regarding why an optimization was rejected
void TerminalReporter::printExplanation(const DiagnosticResult &D) {
  OS << "\n";
  printSeparator('-');
  if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::CYAN);
  OS << "  ROOT CAUSE\n";
  if (Cfg.UseColor) OS.resetColor();
  printSeparator('-');
  OS << "  " << D.RootCause << "\n";

  OS << "\n";
  printSeparator('-');
  if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::CYAN);
  OS << "  WHAT THE OPTIMIZER WANTED TO DO\n";
  if (Cfg.UseColor) OS.resetColor();
  printSeparator('-');
  OS << "  " << D.WhatOptimizerWanted << "\n";

  if (Cfg.Verbose) {
    OS << "\n";
    printSeparator('-');
    if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::CYAN);
    OS << "  DETAILED EXPLANATION\n";
    if (Cfg.UseColor) OS.resetColor();
    printSeparator('-');

    llvm::StringRef Text = D.DetailedExplanation;
    while (!Text.empty()) {
      size_t Newline = Text.find('\n');
      llvm::StringRef Line = Newline == llvm::StringRef::npos
                                 ? Text
                                 : Text.substr(0, Newline);
      if (!Line.empty())
        OS << "  " << Line << "\n";
      else
        OS << "\n";
      if (Newline == llvm::StringRef::npos)
        break;
      Text = Text.substr(Newline + 1);
    }
  }
}

// enumerates actionable codebase modifications to resolve the specific optimization barrier
void TerminalReporter::printSuggestions(const DiagnosticResult &D) {
  if (D.Suggestions.empty())
    return;

  OS << "\n";
  printSeparator('-');
  if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::GREEN, true);
  OS << "  HOW TO FIX THIS\n";
  if (Cfg.UseColor) OS.resetColor();
  printSeparator('-');

  unsigned Count = 0;
  for (const FixSuggestion &Fix : D.Suggestions) {
    if (Count >= Cfg.MaxSuggestions)
      break;
    ++Count;

    OS << "\n  ";
    if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::YELLOW);
    OS << "  " << Count << ". ";
    if (Cfg.UseColor) OS.resetColor();
    OS << Fix.Description << "\n";

    if (!Fix.CodeExample.empty()) {
      OS << "\n";
      if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::WHITE);
      llvm::StringRef Code = Fix.CodeExample;
      while (!Code.empty()) {
        size_t Newline = Code.find('\n');
        llvm::StringRef Line = Newline == llvm::StringRef::npos
                                   ? Code
                                   : Code.substr(0, Newline);
        OS << "      | " << Line << "\n";
        if (Newline == llvm::StringRef::npos)
          break;
        Code = Code.substr(Newline + 1);
      }
      if (Cfg.UseColor) OS.resetColor();
    }

    if (Fix.IsIRLevel) {
      OS << "     ";
      if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::CYAN);
      OS << " [IR-level change]";
      if (Cfg.UseColor) OS.resetColor();
      OS << "\n";
    }
  }
}

// visualizes the specific low-level block and instruction changes caused by optimization attempts
void TerminalReporter::printIRDiff(const FunctionDiff &Diff) {
  if (!Cfg.ShowDiff)
    return;

  OS << "\n";
  printSeparator('-');
  if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::CYAN);
  OS << "  IR DIFF for @" << Diff.FunctionName << "\n";
  if (Cfg.UseColor) OS.resetColor();
  printSeparator('-');

  OS << "  blocks: " << Diff.BeforeBlockCount << " -> " << Diff.AfterBlockCount
     << "   instructions: " << Diff.BeforeInstrCount << " -> "
     << Diff.AfterInstrCount << "\n\n";

  for (const BlockDiff &BD : Diff.Blocks) {
    if (BD.Kind == DiffKind::Unchanged)
      continue;

    OS << "  %" << BD.BlockName << ":\n";

    for (const InstructionDiff &ID : BD.Instructions) {
      switch (ID.Kind) {
      case DiffKind::Unchanged:
        if (Cfg.Verbose)
          OS << "    = " << ID.Before.Text << "\n";
        break;
      case DiffKind::Added:
        if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::GREEN);
        OS << "    + " << ID.After.Text << "\n";
        if (Cfg.UseColor) OS.resetColor();
        break;
      case DiffKind::Removed:
        if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::RED);
        OS << "    - " << ID.Before.Text << "\n";
        if (Cfg.UseColor) OS.resetColor();
        break;
      case DiffKind::Modified:
        if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::RED);
        OS << "    - " << ID.Before.Text << "\n";
        if (Cfg.UseColor) OS.resetColor();
        if (Cfg.UseColor) OS.changeColor(llvm::raw_ostream::GREEN);
        OS << "    + " << ID.After.Text << "\n";
        if (Cfg.UseColor) OS.resetColor();
        break;
      }
    }
  }
}

// encapsulates printing of a single diagnostic, respecting severity thresholds
void TerminalReporter::printDiagnostic(const DiagnosticResult &D) {
  if (static_cast<int>(D.Severity) > static_cast<int>(Cfg.MinSeverity))
    return;

  printDiagnosticHeader(D);
  printExplanation(D);
  printSuggestions(D);

  if (Cfg.ShowDiff && D.IRDiff) {
    printIRDiff(*D.IRDiff);
  }

  OS << "\n";
}

// implements a stable sort to order diagnostics from highest to lowest severity
void TerminalReporter::sortAndFilter(
    std::vector<DiagnosticResult> &Results) const {
  std::stable_sort(
      Results.begin(), Results.end(),
      [](const DiagnosticResult &A, const DiagnosticResult &B) {
        return static_cast<int>(A.Severity) < static_cast<int>(B.Severity);
      });
}

// renders the trailing execution summary and hints for terminal output
void TerminalReporter::printFooter(const AnalysisSession &Session) {
  printSeparator('=');
  OS << "  Total diagnostics: " << Session.Diagnostics.size() << "\n";
  OS << "  Run with --verbose for full explanations\n";
  OS << "  Run with --html=report.html for an interactive report\n";
  printSeparator('=');
  OS << "\n";
}

// main entrypoint for generating a complete terminal-based diagnostic report
void TerminalReporter::report(const AnalysisSession &Session) {
  printHeader(Session);
  printSummaryStats(Session);

  if (Session.Diagnostics.empty()) {
    printColoredLine("  No missed optimizations found for the specified passes.",
                     llvm::raw_ostream::GREEN);
    printFooter(Session);
    return;
  }

  auto Diagnostics = Session.Diagnostics;
  sortAndFilter(Diagnostics);

  for (const DiagnosticResult &D : Diagnostics) {
    if (Cfg.ShowOnlyMissed && !D.PassName.empty())
      printDiagnostic(D);
    else
      printDiagnostic(D);
  }

  printFooter(Session);
}

HTMLReporter::HTMLReporter(llvm::raw_ostream &OS) : OS(OS) {}

// sanitizes raw string data for safe inclusion in an html dom
std::string HTMLReporter::escapeHTML(llvm::StringRef S) {
  std::string Result;
  Result.reserve(S.size());
  for (char C : S) {
    switch (C) {
    case '&':  Result += "&amp;";  break;
    case '<':  Result += "&lt;";   break;
    case '>':  Result += "&gt;";   break;
    case '"':  Result += "&quot;"; break;
    case '\'': Result += "&#39;";  break;
    default:   Result += C;        break;
    }
  }
  return Result;
}

// maps severity levels to specific css class names for styling html elements
std::string HTMLReporter::severityToHTMLClass(SeverityLevel S) {
  switch (S) {
  case SeverityLevel::Critical: return "sev-critical";
  case SeverityLevel::High:     return "sev-high";
  case SeverityLevel::Medium:   return "sev-medium";
  case SeverityLevel::Low:      return "sev-low";
  case SeverityLevel::Info:     return "sev-info";
  }
  return "sev-info";
}

// injects foundational html structures including dark-mode css variables
void HTMLReporter::emitHeader(llvm::StringRef Title) {
  OS << R"html(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>)html" << escapeHTML(Title) << R"html(</title>
<style>
  :root {
    --bg: #0b0e14; --surface: #151921; --surface-alt: #1c212b;
    --border: #2d333b; --border-bright: #444c56;
    --text: #adbac7; --text-muted: #768390; --text-bright: #cdd9e5;
    --red: #e5534b; --yellow: #d29922; --green: #57ab5a;
    --blue: #539bf5; --purple: #b083f0; --cyan: #39c5cf;
    --cobalt: #2e5bff;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { background: var(--bg); color: var(--text); font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif; line-height: 1.5; padding: 0; }
  
  .sidebar { width: 300px; position: fixed; top: 0; bottom: 0; left: 0; background: var(--surface); border-right: 1px solid var(--border); overflow-y: auto; padding: 1.5rem; }
  .main { margin-left: 300px; padding: 2rem 3rem; }
  
  .brand { display: flex; align-items: center; gap: 0.75rem; margin-bottom: 2rem; }
  .brand-logo { width: 32px; height: 32px; border: 2px solid var(--cobalt); border-radius: 4px; display: flex; align-items: center; justify-content: center; font-weight: bold; color: var(--cobalt); font-size: 1.2rem; }
  .brand-name { font-size: 1.1rem; font-weight: 700; color: var(--text-bright); letter-spacing: -0.02em; }
  
  h1 { font-size: 1.5rem; font-weight: 600; color: var(--text-bright); margin-bottom: 0.5rem; }
  .report-meta { font-size: 0.85rem; color: var(--text-muted); margin-bottom: 2rem; font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas, Liberation Mono, monospace; }
  
  .stat-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 1rem; margin-bottom: 2.5rem; }
  .stat-card { background: var(--surface-alt); border: 1px solid var(--border); padding: 1rem; border-radius: 6px; }
  .stat-label { font-size: 0.75rem; font-weight: 600; text-transform: uppercase; color: var(--text-muted); letter-spacing: 0.05em; margin-bottom: 0.5rem; }
  .stat-value { font-size: 1.4rem; font-weight: 700; color: var(--text-bright); font-family: ui-monospace, SFMono-Regular, monospace; }
  
  .nav-item { display: block; padding: 0.5rem 0.75rem; border-radius: 4px; color: var(--text-muted); text-decoration: none; font-size: 0.85rem; margin-bottom: 0.25rem; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
  .nav-item:hover { background: var(--surface-alt); color: var(--text-bright); }
  .nav-group-label { font-size: 0.7rem; font-weight: 700; text-transform: uppercase; color: var(--border-bright); margin: 1.5rem 0 0.5rem 0.75rem; letter-spacing: 0.1em; }
  
  .card { background: var(--surface); border: 1px solid var(--border); border-radius: 8px; margin-bottom: 2rem; overflow: hidden; }
  .card-header { padding: 1rem 1.5rem; background: var(--surface-alt); border-bottom: 1px solid var(--border); display: flex; align-items: center; gap: 1rem; }
  .card-body { padding: 1.5rem; }
  
  .severity-indicator { width: 8px; height: 8px; border-radius: 50%; }
  .sev-critical-dot { background: var(--red); box-shadow: 0 0 8px var(--red); }
  .sev-high-dot     { background: var(--yellow); }
  .sev-medium-dot   { background: var(--purple); }
  .sev-low-dot      { background: var(--cyan); }
  
  .diag-title { font-weight: 600; color: var(--text-bright); font-size: 1rem; }
  .diag-loc { font-family: ui-monospace, SFMono-Regular, monospace; font-size: 0.8rem; color: var(--text-muted); }
  
  .label-group { display: flex; gap: 0.5rem; margin-bottom: 1rem; }
  .label { font-size: 0.7rem; font-weight: 700; padding: 0.15rem 0.4rem; border-radius: 3px; background: var(--border); color: var(--text-muted); text-transform: uppercase; }
  
  .content-section { margin-bottom: 1.5rem; }
  .content-label { font-size: 0.7rem; font-weight: 700; text-transform: uppercase; color: var(--blue); margin-bottom: 0.5rem; letter-spacing: 0.05em; }
  .content-text { font-size: 0.95rem; color: var(--text); line-height: 1.6; }
  
  .fix-container { background: #1c2433; border: 1px solid #3d4d6b; border-radius: 6px; padding: 1rem; border-left: 4px solid var(--blue); }
  .fix-item { margin-bottom: 1rem; }
  .fix-item:last-child { margin-bottom: 0; }
  .fix-desc { font-size: 0.9rem; font-weight: 600; color: var(--text-bright); margin-bottom: 0.5rem; }
  
  pre { font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas, Liberation Mono, monospace; font-size: 0.85rem; padding: 1rem; background: #0b0e14; border-radius: 4px; overflow-x: auto; border: 1px solid var(--border); margin-top: 0.5rem; }
  
  .diff-table { width: 100%; border-collapse: collapse; font-family: ui-monospace, SFMono-Regular, monospace; font-size: 0.8rem; }
  .diff-row:hover { background: #1c212b; }
  .diff-ln { width: 40px; text-align: right; padding-right: 1rem; color: var(--text-muted); user-select: none; border-right: 1px solid var(--border); }
  .diff-content { padding-left: 1rem; white-space: pre; }
  .diff-plus { color: var(--green); background: #1b2e1e; }
  .diff-minus { color: var(--red); background: #351a1a; }
  .diff-meta { color: var(--blue); background: #161b22; font-weight: bold; }
  
  ::-webkit-scrollbar { width: 10px; height: 10px; }
  ::-webkit-scrollbar-track { background: var(--bg); }
  ::-webkit-scrollbar-thumb { background: var(--border); border-radius: 5px; }
  ::-webkit-scrollbar-thumb:hover { background: var(--border-bright); }
</style>
</head>
<body>
)html";
}

// closes top-level html tags
void HTMLReporter::emitFooter() {
  OS << R"html(
</body></html>
)html";
}

// generates a visual dashboard of aggregated performance statistics
void HTMLReporter::emitSummary(const AnalysisSession &Session) {
  const ModuleDiff &D = Session.Diff;

  size_t Missed = 0, Applied = 0;
  for (const Remark &R : Session.Remarks) {
    if (R.isMissed()) ++Missed;
    else if (R.isApplied()) ++Applied;
  }

  OS << "<div class=\"stat-grid\">\n";
  OS << "  <div class=\"stat-card\"><div class=\"stat-label\">Remarks</div><div class=\"stat-value\">"
     << Session.Remarks.size() << "</div></div>\n";
  OS << "  <div class=\"stat-card\"><div class=\"stat-label\">Missed Opts</div><div class=\"stat-value\" style=\"color:var(--red)\">"
     << Missed << "</div></div>\n";
  OS << "  <div class=\"stat-card\"><div class=\"stat-label\">Applied</div><div class=\"stat-value\" style=\"color:var(--green)\">"
     << Applied << "</div></div>\n";
  OS << "  <div class=\"stat-card\"><div class=\"stat-label\">Functions</div><div class=\"stat-value\">"
     << (D.ModifiedFunctions + D.UnchangedFunctions) << "</div></div>\n";
  OS << "  <div class=\"stat-card\"><div class=\"stat-label\">Instr Delta</div><div class=\"stat-value\">"
     << D.instructionDelta() << "</div></div>\n";
  OS << "</div>\n";
}

// translates structural ir diff data into an interactive html table
void HTMLReporter::emitIRDiff(const FunctionDiff &Diff) {
  OS << "<div class=\"content-section\">\n";
  OS << "  <div class=\"content-label\">Structural IR Changes</div>\n";
  OS << "  <div class=\"card\">\n";
  OS << "    <table class=\"diff-table\">\n";

  for (const BlockDiff &BD : Diff.Blocks) {
    if (BD.Kind == DiffKind::Unchanged) continue;
    OS << "      <tr class=\"diff-row\"><td class=\"diff-ln\">#</td><td class=\"diff-content diff-meta\">%" 
       << escapeHTML(BD.BlockName) << ":</td></tr>\n";

    unsigned LineIdx = 0;
    for (const InstructionDiff &ID : BD.Instructions) {
      ++LineIdx;
      std::string LineStr = std::to_string(LineIdx);
      
      switch (ID.Kind) {
      case DiffKind::Unchanged:
        OS << "      <tr class=\"diff-row\"><td class=\"diff-ln\">" << LineStr << "</td><td class=\"diff-content\">  " 
           << escapeHTML(ID.Before.Text) << "</td></tr>\n";
        break;
      case DiffKind::Added:
        OS << "      <tr class=\"diff-row diff-plus\"><td class=\"diff-ln\">+</td><td class=\"diff-content\">  " 
           << escapeHTML(ID.After.Text) << "</td></tr>\n";
        break;
      case DiffKind::Removed:
        OS << "      <tr class=\"diff-row diff-minus\"><td class=\"diff-ln\">-</td><td class=\"diff-content\">  " 
           << escapeHTML(ID.Before.Text) << "</td></tr>\n";
        break;
      case DiffKind::Modified:
        OS << "      <tr class=\"diff-row diff-minus\"><td class=\"diff-ln\">-</td><td class=\"diff-content\">  " 
           << escapeHTML(ID.Before.Text) << "</td></tr>\n";
        OS << "      <tr class=\"diff-row diff-plus\"><td class=\"diff-ln\">+</td><td class=\"diff-content\">  " 
           << escapeHTML(ID.After.Text) << "</td></tr>\n";
        break;
      }
    }
  }
  OS << "    </table>\n";
  OS << "  </div>\n";
  OS << "</div>\n";
}

// compiles a single missed optimization instance into a comprehensive html card
void HTMLReporter::emitDiagnostic(const DiagnosticResult &D,
                                   const ReportConfig     &Cfg) {
  OS << "<div class=\"card\">\n";
  OS << "  <div class=\"card-header\">\n";
  
  std::string ColorDot = "sev-info-dot";
  switch (D.Severity) {
    case SeverityLevel::Critical: ColorDot = "sev-critical-dot"; break;
    case SeverityLevel::High:     ColorDot = "sev-high-dot"; break;
    case SeverityLevel::Medium:   ColorDot = "sev-medium-dot"; break;
    case SeverityLevel::Low:      ColorDot = "sev-low-dot"; break;
    default: break;
  }
  
  OS << "    <div class=\"severity-indicator " << ColorDot << "\"></div>\n";
  OS << "    <div class=\"diag-title\">" << escapeHTML(D.ShortReason) << "</div>\n";
  if (D.Location.isValid())
    OS << "    <div class=\"diag-loc\">" << escapeHTML(D.Location.format()) << "</div>\n";
  OS << "  </div>\n";
  
  OS << "  <div class=\"card-body\">\n";
  
  OS << "    <div class=\"label-group\">\n";
  OS << "      <div class=\"label\">" << escapeHTML(D.PassName) << "</div>\n";
  OS << "      <div class=\"label\">@" << escapeHTML(D.FunctionName) << "</div>\n";
  if (D.EstimatedSpeedup > 0.1) {
    std::ostringstream SS;
    SS << std::fixed << std::setprecision(1) << D.EstimatedSpeedup;
    OS << "      <div class=\"label\" style=\"color:var(--green)\">Estimated Speedup: " << SS.str() << "x</div>\n";
  }
  OS << "    </div>\n";

  OS << "    <div class=\"content-section\">\n";
  OS << "      <div class=\"content-label\">Root Cause</div>\n";
  OS << "      <div class=\"content-text\">" << escapeHTML(D.RootCause) << "</div>\n";
  OS << "    </div>\n";

  OS << "    <div class=\"content-section\">\n";
  OS << "      <div class=\"content-label\">Optimizer Intent</div>\n";
  OS << "      <div class=\"content-text\">" << escapeHTML(D.WhatOptimizerWanted) << "</div>\n";
  OS << "    </div>\n";

  if (!D.Suggestions.empty()) {
    OS << "    <div class=\"content-label\">Actionable Resolutions</div>\n";
    OS << "    <div class=\"fix-container\">\n";
    unsigned Count = 0;
    for (const FixSuggestion &Fix : D.Suggestions) {
      if (Count >= Cfg.MaxSuggestions) break;
      ++Count;
      OS << "      <div class=\"fix-item\">\n";
      OS << "        <div class=\"fix-desc\">" << Count << ". " << escapeHTML(Fix.Description) << "</div>\n";
      if (!Fix.CodeExample.empty())
        OS << "        <pre>" << escapeHTML(Fix.CodeExample) << "</pre>\n";
      OS << "      </div>\n";
    }
    OS << "    </div>\n";
  }

  if (D.IRDiff) {
    OS << "\n";
    emitIRDiff(*D.IRDiff);
  }

  OS << "  </div>\n";
  OS << "</div>\n";
}

// parses the unified analysis session into a complete standalone interactive html file
void HTMLReporter::report(const AnalysisSession &Session,
                           const ReportConfig    &Cfg) {
  emitHeader("Aion Performance Report");

  OS << "<div class=\"sidebar\">\n";
  OS << "  <div class=\"brand\">\n";
  OS << "    <div class=\"brand-logo\">\n";
  OS << "      <svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2.5\" stroke-linecap=\"square\">\n";
  OS << "        <path d=\"M3 21L12 3L21 21M7 13H17\" />\n";
  OS << "        <path d=\"M2 21H22\" />\n";
  OS << "      </svg>\n";
  OS << "    </div>\n";
  OS << "    <div class=\"brand-name\">AION</div>\n";
  OS << "  </div>\n";
  
  OS << "  <div class=\"nav-group-label\">Navigation</div>\n";
  OS << "  <a href=\"#summary\" class=\"nav-item\">Executive Summary</a>\n";
  
  int Idx = 0;
  if (!Session.Diagnostics.empty()) {
    OS << "  <div class=\"nav-group-label\">Missed Optimizations</div>\n";
    for (const DiagnosticResult &D : Session.Diagnostics) {
      std::string Target = "diag-" + std::to_string(Idx++);
      OS << "  <a href=\"#" << Target << "\" class=\"nav-item\">" 
         << escapeHTML(D.PassName) << ": " << escapeHTML(D.ShortReason) << "</a>\n";
    }
  }
  OS << "</div>\n";

  OS << "<div class=\"main\">\n";
  OS << "  <div id=\"summary\">\n";
  OS << "    <h1>Compiler Optimization Analysis</h1>\n";
  OS << "    <div class=\"report-meta\">Engine: Aion v1.0 // Pipeline: " 
     << escapeHTML(Session.PassPipelineUsed) << "</div>\n";
  emitSummary(Session);
  OS << "  </div>\n";

  Idx = 0;
  for (const DiagnosticResult &D : Session.Diagnostics) {
    std::string Target = "diag-" + std::to_string(Idx++);
    OS << "<div id=\"" << Target << "\"></div>\n";
    emitDiagnostic(D, Cfg);
  }

  OS << "</div>\n";
  emitFooter();
}

// triggers the dual-stage reporting sequence emitting both terminal text and optionally an html dashboard
void generateReport(const AnalysisSession &Session,
                    const ReportConfig    &Cfg,
                    llvm::raw_ostream     &TerminalOS,
                    llvm::StringRef        HTMLOutputPath) {
  TerminalReporter TR(TerminalOS, Cfg);
  TR.report(Session);

  if (HTMLOutputPath.empty())
    return;

  std::error_code EC;
  llvm::raw_fd_ostream HTMLFile(HTMLOutputPath, EC,
                                 llvm::sys::fs::OF_Text);
  if (EC) {
    TerminalOS << "Warning: could not write HTML report to '"
               << HTMLOutputPath << "': " << EC.message() << "\n";
    return;
  }

  HTMLReporter HR(HTMLFile);
  HR.report(Session, Cfg);
  TerminalOS << "HTML report written to: " << HTMLOutputPath << "\n";
}

}
