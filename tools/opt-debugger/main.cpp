#include "OptDebugger/OptReport.h"
#include "OptDebugger/PassAnalyzer.h"
#include "OptDebugger/Support.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/WithColor.h"

using namespace llvm;
using namespace optdbg;

static cl::OptionCategory OptDbgCategory("opt-debugger options");

static cl::opt<std::string> InputFile(
    cl::Positional,
    cl::desc("<input .ll or .bc file>"),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> BeforeFile(
    "before",
    cl::desc("IR file before optimization (use with --after)"),
    cl::value_desc("file.ll"),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> AfterFile(
    "after",
    cl::desc("IR file after optimization (use with --before)"),
    cl::value_desc("file.ll"),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> RemarksFile(
    "remarks",
    cl::desc("Optimization remarks YAML file (from -fsave-optimization-record)"),
    cl::value_desc("remarks.yaml"),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> Passes(
    "passes",
    cl::desc("Pass pipeline to run (same syntax as opt -passes=...). "
             "Example: --passes=inline,loop-vectorize,gvn"),
    cl::value_desc("pipeline"),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> OptLevel(
    "O",
    cl::desc("Optimization level when no explicit pass pipeline is given "
             "(O0, O1, O2, O3, Os, Oz). Default: O2"),
    cl::value_desc("level"),
    cl::init("O2"),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> HTMLOutput(
    "html",
    cl::desc("Write an interactive HTML report to this file"),
    cl::value_desc("report.html"),
    cl::cat(OptDbgCategory));

static cl::opt<bool> Verbose(
    "verbose",
    cl::desc("Print full detailed explanations (not just root cause)"),
    cl::init(false),
    cl::cat(OptDbgCategory));

static cl::opt<bool> NoColor(
    "no-color",
    cl::desc("Disable terminal color output"),
    cl::init(false),
    cl::cat(OptDbgCategory));

static cl::opt<bool> ShowDiff(
    "diff",
    cl::desc("Show IR diff for each affected function"),
    cl::init(true),
    cl::cat(OptDbgCategory));

static cl::opt<bool> ShowOnlyMissed(
    "missed-only",
    cl::desc("Show only missed optimization diagnostics (not applied)"),
    cl::init(true),
    cl::cat(OptDbgCategory));

static cl::opt<unsigned> MaxSuggestions(
    "max-suggestions",
    cl::desc("Maximum number of fix suggestions per diagnostic. Default: 3"),
    cl::init(3),
    cl::cat(OptDbgCategory));

static cl::opt<bool> EnableVectorization(
    "vectorize",
    cl::desc("Enable loop and SLP vectorization in the analysis pipeline"),
    cl::init(true),
    cl::cat(OptDbgCategory));

static cl::opt<bool> EnableUnrolling(
    "unroll",
    cl::desc("Enable loop unrolling in the analysis pipeline"),
    cl::init(true),
    cl::cat(OptDbgCategory));

static cl::opt<bool> VerifyEach(
    "verify-each",
    cl::desc("Run the IR verifier after each pass (slower but finds bugs)"),
    cl::init(false),
    cl::cat(OptDbgCategory));

static cl::opt<std::string> MinSeverity(
    "min-severity",
    cl::desc("Minimum severity level to report: critical, high, medium, low, info"),
    cl::init("low"),
    cl::cat(OptDbgCategory));

static cl::opt<bool> PrintSummaryOnly(
    "summary-only",
    cl::desc("Print only the summary statistics, skip per-diagnostic output"),
    cl::init(false),
    cl::cat(OptDbgCategory));

static cl::opt<bool> GroupByFunction(
    "group-by-function",
    cl::desc("Group diagnostics by function name"),
    cl::init(false),
    cl::cat(OptDbgCategory));

static cl::opt<bool> GroupByPass(
    "group-by-pass",
    cl::desc("Group diagnostics by pass name"),
    cl::init(false),
    cl::cat(OptDbgCategory));

namespace {

// parses string input into the internal severity level enum
SeverityLevel parseSeverityLevel(StringRef S) {
  if (S.equals_insensitive("critical")) return SeverityLevel::Critical;
  if (S.equals_insensitive("high"))     return SeverityLevel::High;
  if (S.equals_insensitive("medium"))   return SeverityLevel::Medium;
  if (S.equals_insensitive("low"))      return SeverityLevel::Low;
  if (S.equals_insensitive("info"))     return SeverityLevel::Info;
  return SeverityLevel::Low;
}

// formats and prints a standard usage error to standard error
void printUsageError(StringRef Msg) {
  WithColor::error(errs(), "opt-debugger") << Msg << "\n";
  errs() << "Run 'opt-debugger --help' for usage information.\n";
}

// validates command line flags for incompatible option combinations
bool hasConflictingOptions() {
  bool HasInput        = !InputFile.empty();
  bool HasBeforeAfter  = !BeforeFile.empty() && !AfterFile.empty();
  bool HasBeforeOnly   = !BeforeFile.empty() && AfterFile.empty();
  bool HasAfterOnly    = BeforeFile.empty() && !AfterFile.empty();

  if (HasBeforeOnly) {
    printUsageError("--before requires --after");
    return true;
  }

  if (HasAfterOnly) {
    printUsageError("--after requires --before");
    return true;
  }

  if (HasInput && HasBeforeAfter) {
    printUsageError("Cannot specify both a positional input file and --before/--after");
    return true;
  }

  if (!HasInput && !HasBeforeAfter) {
    printUsageError("No input specified. Provide an IR file or use --before/--after");
    return true;
  }

  return false;
}

}

// entry point for the opt-debugger executable
int main(int argc, char **argv) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(OptDbgCategory);
  cl::ParseCommandLineOptions(
      argc, argv,
      "opt-debugger: Why wasn't my code optimized?\n\n"
      "Analyzes LLVM IR to determine why optimization passes\n"
      "rejected transformations, providing technical diagnostics\n"
      "and actionable resolutions.\n\n"
      "Examples:\n"
      "  opt-debugger input.ll\n"
      "  opt-debugger input.ll --passes=inline,loop-vectorize\n"
      "  opt-debugger --before=before.ll --after=after.ll --remarks=r.yaml\n"
      "  opt-debugger input.ll -O3 --html=report.html --verbose\n");

  if (hasConflictingOptions())
    return 1;

  bool UseColor = !NoColor && llvm::sys::Process::StandardOutIsDisplayed();

  ReportConfig RCfg;
  RCfg.ShowDiff        = ShowDiff;
  RCfg.ShowSuggestions = true;
  RCfg.ShowIRSnippets  = Verbose;
  RCfg.UseColor        = UseColor;
  RCfg.Verbose         = Verbose;
  RCfg.ShowOnlyMissed  = ShowOnlyMissed;
  RCfg.GroupByPass     = GroupByPass;
  RCfg.GroupByFunction = GroupByFunction;
  RCfg.MaxSuggestions  = MaxSuggestions;
  RCfg.MinSeverity     = parseSeverityLevel(MinSeverity);

  PassAnalyzer Analyzer;

  Expected<AnalysisSession> SessionOrErr = [&]() -> Expected<AnalysisSession> {
    if (!BeforeFile.empty()) {
      return Analyzer.runFromBeforeAfter(BeforeFile, AfterFile, RemarksFile);
    }

    AnalysisConfig ACfg;
    ACfg.PassPipeline        = Passes;
    ACfg.OptLevel            = OptLevel;
    ACfg.EnableAllRemarks    = true;
    ACfg.EnableVectorization = EnableVectorization;
    ACfg.EnableUnrolling     = EnableUnrolling;
    ACfg.VerifyEachPass      = VerifyEach;

    return Analyzer.runFromFile(InputFile, ACfg);
  }();

  if (!SessionOrErr) {
    WithColor::error(errs(), "opt-debugger")
        << toString(SessionOrErr.takeError()) << "\n";
    return 1;
  }

  AnalysisSession &Session = *SessionOrErr;

  if (PrintSummaryOnly) {
    ReportConfig SummaryCfg = RCfg;
    SummaryCfg.ShowDiff       = false;
    SummaryCfg.ShowSuggestions = false;
    SummaryCfg.ShowIRSnippets  = false;
    TerminalReporter TR(outs(), SummaryCfg);
    TR.report(Session);
  } else {
    generateReport(Session, RCfg, outs(), HTMLOutput);
  }

  bool HadCritical = false;
  for (const DiagnosticResult &D : Session.Diagnostics)
    if (D.Severity == SeverityLevel::Critical)
      HadCritical = true;

  return HadCritical ? 2 : 0;
}
