#pragma once

#include "OptDebugger/DiagnosticEngine.h"
#include "OptDebugger/PassAnalyzer.h"
#include "OptDebugger/Support.h"
#include "llvm/Support/raw_ostream.h"

namespace optdbg {

struct ReportConfig {
  bool ShowDiff = true;
  bool ShowSuggestions = true;
  bool ShowIRSnippets = false;
  bool UseColor = true;
  bool Verbose = false;
  bool ShowOnlyMissed = true;
  bool GroupByPass = false;
  bool GroupByFunction = false;
  unsigned MaxSuggestions = 3;
  SeverityLevel MinSeverity = SeverityLevel::Low;
};

class TerminalReporter {
public:
  TerminalReporter(llvm::raw_ostream &OS, ReportConfig Config);
  void report(const AnalysisSession &Session);

private:
  void printSeparator(char Ch = '=', unsigned Width = 80);
  void printColoredLine(llvm::StringRef Text, llvm::raw_ostream::Colors Color);
  void printHeader(const AnalysisSession &Session);
  void printSummaryStats(const AnalysisSession &Session);
  void printDiagnostic(const DiagnosticResult &D);
  void printDiagnosticHeader(const DiagnosticResult &D);
  void printExplanation(const DiagnosticResult &D);
  void printSuggestions(const DiagnosticResult &D);
  void printIRDiff(const FunctionDiff &Diff);
  void printFooter(const AnalysisSession &Session);
  void sortAndFilter(std::vector<DiagnosticResult> &Results) const;

  llvm::raw_ostream &OS;
  ReportConfig Cfg;
};

class HTMLReporter {
public:
  HTMLReporter(llvm::raw_ostream &OS);
  void report(const AnalysisSession &Session, const ReportConfig &Cfg);

private:
  void emitHeader(llvm::StringRef Title);
  void emitSummary(const AnalysisSession &Session);
  void emitDiagnostic(const DiagnosticResult &D, const ReportConfig &Cfg);
  void emitIRDiff(const FunctionDiff &Diff);
  void emitFooter();

  static std::string escapeHTML(llvm::StringRef S);
  static std::string severityToHTMLClass(SeverityLevel S);

  llvm::raw_ostream &OS;
};

void generateReport(const AnalysisSession &Session,
                    const ReportConfig    &Cfg,
                    llvm::raw_ostream     &TerminalOS,
                    llvm::StringRef        HTMLOutputPath);

}
