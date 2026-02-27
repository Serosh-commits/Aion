#pragma once

#include "OptDebugger/DiagnosticEngine.h"
#include "OptDebugger/IRDiff.h"
#include "OptDebugger/RemarkCollector.h"
#include "OptDebugger/Support.h"

#include "llvm/IR/Module.h"
#include "llvm/Support/Error.h"

#include <memory>
#include <string>

namespace optdbg {

struct AnalysisConfig {
  std::string   PassPipeline;
  std::string   OptLevel;
  bool          EnableAllRemarks    = true;
  bool          EnableHotnessInfo   = false;
  bool          VerifyEachPass      = false;
  bool          PrintPassStructure  = false;
  unsigned      InlineThreshold     = 225;
  bool          EnableVectorization = true;
  bool          EnableUnrolling     = true;
};

struct AnalysisSession {
  std::vector<std::unique_ptr<llvm::LLVMContext>> Contexts;
  std::unique_ptr<llvm::Module> BeforeModule;
  std::unique_ptr<llvm::Module> AfterModule;
  std::string                   BeforeIR;
  std::string                   AfterIR;
  std::vector<Remark>           Remarks;
  ModuleDiff                    Diff;
  std::vector<DiagnosticResult> Diagnostics;
  std::string                   PassPipelineUsed;
  bool                          VerificationFailed = false;
};

class PassAnalyzer {
public:
  PassAnalyzer();
  ~PassAnalyzer() = default;

  PassAnalyzer(const PassAnalyzer &)            = delete;
  PassAnalyzer &operator=(const PassAnalyzer &) = delete;

  llvm::Expected<AnalysisSession>
  runFromFile(llvm::StringRef InputPath, const AnalysisConfig &Config);

  llvm::Expected<AnalysisSession>
  runFromIR(llvm::StringRef IRText, const AnalysisConfig &Config);

  llvm::Expected<AnalysisSession>
  runFromBeforeAfter(llvm::StringRef BeforePath,
                     llvm::StringRef AfterPath,
                     llvm::StringRef RemarksYAMLPath);

  llvm::Expected<AnalysisSession>
  runFromModules(std::unique_ptr<llvm::Module> Before,
                 std::unique_ptr<llvm::Module> After,
                 std::vector<Remark>           ExternalRemarks);

private:
  llvm::Expected<AnalysisSession>
  executeAnalysis(std::unique_ptr<llvm::Module> BeforeModule,
                  const AnalysisConfig          &Config);

  llvm::Error runPassPipeline(llvm::Module             &M,
                               const AnalysisConfig     &Config,
                               RemarkCollector          &Collector);

  static std::string moduleToString(const llvm::Module &M);

  static llvm::Expected<std::unique_ptr<llvm::Module>>
  parseIRFromFile(llvm::StringRef Path, llvm::LLVMContext &Ctx);

  static llvm::Expected<std::unique_ptr<llvm::Module>>
  parseIRFromString(llvm::StringRef IRText, llvm::LLVMContext &Ctx);

  static llvm::Expected<std::vector<Remark>>
  parseRemarksYAML(llvm::StringRef Path);

  static llvm::Error verifyModule(const llvm::Module &M);

  IRDiffEngine   DiffEngine;
  DiagnosticEngine DiagEngine;
};

}
