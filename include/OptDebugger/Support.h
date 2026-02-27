#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <string>
#include <vector>
#include <optional>

namespace optdbg {

struct SourceLocation {
  std::string File;
  unsigned Line = 0;
  unsigned Column = 0;

  bool isValid() const { return !File.empty(); }
  std::string format() const {
    if (!isValid()) return "<unknown>";
    return File + ":" + std::to_string(Line) + ":" + std::to_string(Column);
  }
};

enum class RemarkKind {
  Applied,
  Missed,
  Analysis,
  AnalysisAliasing,
  AnalysisFPCommute
};

struct RemarkArgument {
  std::string Key;
  std::string Value;
  SourceLocation Loc;
};

struct Remark {
  RemarkKind Kind;
  std::string PassName;
  std::string RemarkName;
  std::string FunctionName;
  SourceLocation Loc;
  std::string Message;
  std::vector<RemarkArgument> Args;
  std::optional<float> Hotness;

  bool isMissed() const { return Kind == RemarkKind::Missed; }
  bool isApplied() const { return Kind == RemarkKind::Applied; }
  bool isAnalysis() const {
    return Kind == RemarkKind::Analysis ||
           Kind == RemarkKind::AnalysisAliasing ||
           Kind == RemarkKind::AnalysisFPCommute;
  }
};

inline llvm::Error makeStringError(const llvm::Twine &Msg) {
  return llvm::make_error<llvm::StringError>(Msg, llvm::inconvertibleErrorCode());
}

bool matchesPattern(llvm::StringRef Text, llvm::StringRef Pattern);

}
