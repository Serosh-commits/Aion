#pragma once

#include "OptDebugger/IRDiff.h"
#include "OptDebugger/Support.h"

#include "llvm/ADT/StringRef.h"

#include <optional>
#include <string>
#include <vector>

namespace optdbg {

enum class SeverityLevel : uint8_t {
  Critical,
  High,
  Medium,
  Low,
  Info,
};

struct FixSuggestion {
  std::string Description;
  std::string CodeExample;
  bool        IsSourceLevel;
  bool        IsIRLevel;
};

struct DiagnosticResult {
  std::string               PassName;
  std::string               FunctionName;
  SourceLocation            Location;
  std::string               ShortReason;
  std::string               DetailedExplanation;
  std::string               RootCause;
  std::string               WhatOptimizerWanted;
  std::vector<FixSuggestion> Suggestions;
  SeverityLevel             Severity;
  std::optional<FunctionDiff> IRDiff;
  double                    EstimatedSpeedup;

  bool hasFix() const { return !Suggestions.empty(); }
};

struct OptimizationPattern {
  llvm::StringRef PassNameSubstr;
  llvm::StringRef RemarkNameSubstr;
  llvm::StringRef MessageSubstr;
  std::string     ShortReason;
  std::string     DetailedExplanation;
  std::string     RootCause;
  std::string     WhatOptimizerWanted;
  std::vector<FixSuggestion> Suggestions;
  SeverityLevel   Severity;
  double          EstimatedSpeedup;
};

class DiagnosticEngine {
public:
  DiagnosticEngine();
  ~DiagnosticEngine() = default;

  DiagnosticEngine(const DiagnosticEngine &)            = delete;
  DiagnosticEngine &operator=(const DiagnosticEngine &) = delete;

  std::vector<DiagnosticResult>
  analyze(const std::vector<Remark> &Remarks,
          const ModuleDiff          &Diff) const;

  DiagnosticResult
  analyzeRemark(const Remark              &R,
                const ModuleDiff          &Diff) const;

private:
  std::vector<OptimizationPattern> Patterns;

  void registerPatterns();

  void registerInliningPatterns();
  void registerLoopVectorizationPatterns();
  void registerSLPVectorizationPatterns();
  void registerSROAPatterns();
  void registerLoopUnrollPatterns();
  void registerTailCallPatterns();
  void registerGVNPatterns();
  void registerMemCpyOptPatterns();
  void registerLoopInterchangePatterns();
  void registerGenericPatterns();

  const OptimizationPattern *
  findMatchingPattern(const Remark &R) const;

  DiagnosticResult
  buildFromPattern(const Remark              &R,
                   const OptimizationPattern &P) const;

  DiagnosticResult
  buildFallback(const Remark &R) const;

  DiagnosticResult
  analyzeRemark(const Remark &R) const;

  static std::string interpolateArgs(const std::string &Template,
                                      const Remark      &R);
};

llvm::StringRef severityToString(SeverityLevel S);
llvm::StringRef severityToEmoji(SeverityLevel S);

}
