#include "OptDebugger/PassAnalyzer.h"
#include "OptDebugger/Support.h"

#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/LastRunTrackingAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <sstream>
#include <utility>

namespace optdbg {

PassAnalyzer::PassAnalyzer() = default;

// serializes an llvm module in memory back to an ir string representation
std::string PassAnalyzer::moduleToString(const llvm::Module &M) {
  std::string S;
  llvm::raw_string_ostream OS(S);
  M.print(OS, nullptr);
  OS.flush();
  return S;
}

// parses an llvm ir file from the filesystem into an in-memory module
llvm::Expected<std::unique_ptr<llvm::Module>>
PassAnalyzer::parseIRFromFile(llvm::StringRef Path,
                               llvm::LLVMContext &Ctx) {
  llvm::SMDiagnostic Err;
  auto M = llvm::parseIRFile(Path, Err, Ctx);
  if (!M) {
    std::string Msg;
    llvm::raw_string_ostream OS(Msg);
    Err.print("opt-debugger", OS);
    OS.flush();
    return makeStringError("Failed to parse IR file '" + Path + "': " + Msg);
  }
  return M;
}

// parses an llvm ir text string directly into an in-memory module
llvm::Expected<std::unique_ptr<llvm::Module>>
PassAnalyzer::parseIRFromString(llvm::StringRef IRText,
                                 llvm::LLVMContext &Ctx) {
  llvm::SMDiagnostic Err;
  auto Buf = llvm::MemoryBuffer::getMemBuffer(IRText, "<string>");
  auto M   = llvm::parseIR(*Buf, Err, Ctx);
  if (!M) {
    std::string Msg;
    llvm::raw_string_ostream OS(Msg);
    Err.print("opt-debugger", OS);
    OS.flush();
    return makeStringError("Failed to parse IR string: " + Msg);
  }
  return M;
}

// runs the full suite of llvm structural verifications against a given module
llvm::Error PassAnalyzer::verifyModule(const llvm::Module &M) {
  std::string ErrorStr;
  llvm::raw_string_ostream ES(ErrorStr);
  if (llvm::verifyModule(M, &ES)) {
    ES.flush();
    return makeStringError("Module verification failed: " + ErrorStr);
  }
  return llvm::Error::success();
}

llvm::Error PassAnalyzer::runPassPipeline(llvm::Module         &M,
                                           const AnalysisConfig &Config,
                                           RemarkCollector      &Collector) {
  // wire up our custom diagnostic handler to capture optimization remarks emitted during passes.
  Collector.install(M.getContext());

  // instantiate the individual analysis managers required by the llvm new pass manager.
  llvm::LoopAnalysisManager   LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager    CGAM;
  llvm::ModuleAnalysisManager   MAM;

  // register all foundational function-level analyses like dominator trees and basic alias analysis.
  FAM.registerPass([&] { return llvm::PassInstrumentationAnalysis(); });
  FAM.registerPass([&] { return llvm::DominatorTreeAnalysis(); });
  FAM.registerPass([&] { return llvm::TargetLibraryAnalysis(); });
  FAM.registerPass([&] { return llvm::TargetIRAnalysis(); });
  FAM.registerPass([&] { return llvm::LastRunTrackingAnalysis(); });
  FAM.registerPass([&] { return llvm::AssumptionAnalysis(); });
  FAM.registerPass([&] { return llvm::OptimizationRemarkEmitterAnalysis(); });
  FAM.registerPass([&] { return llvm::BranchProbabilityAnalysis(); });
  FAM.registerPass([&] { return llvm::PostDominatorTreeAnalysis(); });
  FAM.registerPass([&] { return llvm::AAManager(); });
  FAM.registerPass([&] { return llvm::LoopAnalysisManagerFunctionProxy(LAM); });
  FAM.registerPass([&] { return llvm::ModuleAnalysisManagerFunctionProxy(MAM); });

  // register broader module-level analyses and set up the proxy linking back to function analyses.
  MAM.registerPass([&] { return llvm::PassInstrumentationAnalysis(); });
  MAM.registerPass([&] { return llvm::ProfileSummaryAnalysis(); });
  MAM.registerPass([&] { return llvm::FunctionAnalysisManagerModuleProxy(FAM); });

  // register specialized loop analyses and cross-link them with the function analysis manager.
  LAM.registerPass([&] { return llvm::PassInstrumentationAnalysis(); });
  LAM.registerPass([&] { return llvm::FunctionAnalysisManagerLoopProxy(FAM); });

  // build the core sequence of transformation passes based on the user's configuration string.
  llvm::FunctionPassManager FPM;
  
  if (!Config.PassPipeline.empty()) {
    llvm::StringRef PipelineStr = Config.PassPipeline;
    if (PipelineStr.contains("instcombine")) FPM.addPass(llvm::InstCombinePass());
    if (PipelineStr.contains("simplifycfg")) FPM.addPass(llvm::SimplifyCFGPass());
    if (PipelineStr.contains("adce")) FPM.addPass(llvm::ADCEPass());
  } else {
    FPM.addPass(llvm::InstCombinePass());
    FPM.addPass(llvm::SimplifyCFGPass());
    FPM.addPass(llvm::ADCEPass());
  }

  // wrap the function pipeline into a module-level manager so it can be executed across the whole ir.
  llvm::ModulePassManager MPM;
  MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

  // kick off the analysis and transformation process.
  MPM.run(M, MAM);
  return llvm::Error::success();
}

// orchestrates the end-to-end analysis by copying modules, running passes, and diffing structural states
llvm::Expected<AnalysisSession>
PassAnalyzer::executeAnalysis(std::unique_ptr<llvm::Module> BeforeModule,
                               const AnalysisConfig          &Config) {
  if (!BeforeModule)
    return makeStringError("Null input module");

  for (auto &F : *BeforeModule) {
    if (F.isDeclaration()) continue;
    unsigned BBId = 0;
    for (auto &BB : F) {
      if (!BB.hasName())
        BB.setName("aion.bb." + std::to_string(BBId));
      ++BBId;
    }
  }

  if (auto Err = verifyModule(*BeforeModule))
    return std::move(Err);

  AnalysisSession Session;
  Session.PassPipelineUsed = Config.PassPipeline.empty()
                                 ? ("default-" + Config.OptLevel)
                                 : Config.PassPipeline;

  Session.BeforeIR = moduleToString(*BeforeModule);

  auto AfterModule = llvm::CloneModule(*BeforeModule);
  if (!AfterModule)
    return makeStringError("Failed to clone module for analysis");

  RemarkCollector Collector;
  if (auto Err = runPassPipeline(*AfterModule, Config, Collector))
    return std::move(Err);

  if (Config.VerifyEachPass) {
    if (auto Err = verifyModule(*AfterModule)) {
      Session.VerificationFailed = true;
      return std::move(Err);
    }
  }

  Session.AfterIR  = moduleToString(*AfterModule);
  Session.Remarks  = Collector.getRemarks();

  Session.Diff        = DiffEngine.diff(*BeforeModule, *AfterModule);
  Session.Diagnostics = DiagEngine.analyze(Session.Remarks, Session.Diff);

  Session.BeforeModule = std::move(BeforeModule);
  Session.AfterModule  = std::move(AfterModule);

  return Session;
}

// wrapper to instantiate context, parse ir from a file, and execute the analysis pipeline
llvm::Expected<AnalysisSession>
PassAnalyzer::runFromFile(llvm::StringRef InputPath,
                           const AnalysisConfig &Config) {
  auto Ctx = std::make_unique<llvm::LLVMContext>();
  auto ModuleOrErr = parseIRFromFile(InputPath, *Ctx);
  if (!ModuleOrErr)
    return ModuleOrErr.takeError();
  auto SessionOrErr = executeAnalysis(std::move(*ModuleOrErr), Config);
  if (SessionOrErr)
    SessionOrErr->Contexts.push_back(std::move(Ctx));
  return SessionOrErr;
}

// wrapper to instantiate context, parse ir from an in-memory string, and execute the analysis pipeline
llvm::Expected<AnalysisSession>
PassAnalyzer::runFromIR(llvm::StringRef IRText,
                         const AnalysisConfig &Config) {
  auto Ctx = std::make_unique<llvm::LLVMContext>();
  auto ModuleOrErr = parseIRFromString(IRText, *Ctx);
  if (!ModuleOrErr)
    return ModuleOrErr.takeError();
  auto SessionOrErr = executeAnalysis(std::move(*ModuleOrErr), Config);
  if (SessionOrErr)
    SessionOrErr->Contexts.push_back(std::move(Ctx));
  return SessionOrErr;
}

// bypasses optimization and analyzes two existing modules while importing serialized diagnostics
llvm::Expected<AnalysisSession>
PassAnalyzer::runFromBeforeAfter(llvm::StringRef BeforePath,
                                  llvm::StringRef AfterPath,
                                  llvm::StringRef RemarksYAMLPath) {
  auto BeforeCtx = std::make_unique<llvm::LLVMContext>();
  auto AfterCtx  = std::make_unique<llvm::LLVMContext>();

  auto BeforeOrErr = parseIRFromFile(BeforePath, *BeforeCtx);
  if (!BeforeOrErr)
    return BeforeOrErr.takeError();

  auto AfterOrErr = parseIRFromFile(AfterPath, *AfterCtx);
  if (!AfterOrErr)
    return AfterOrErr.takeError();

  std::vector<Remark> Remarks;
  if (!RemarksYAMLPath.empty()) {
    auto RemarksOrErr = parseRemarksYAML(RemarksYAMLPath);
    if (!RemarksOrErr)
      return RemarksOrErr.takeError();
    Remarks = std::move(*RemarksOrErr);
  }

  auto SessionOrErr = runFromModules(std::move(*BeforeOrErr), std::move(*AfterOrErr),
                        std::move(Remarks));
  if (SessionOrErr) {
    SessionOrErr->Contexts.push_back(std::move(BeforeCtx));
    SessionOrErr->Contexts.push_back(std::move(AfterCtx));
  }
  return SessionOrErr;
}

// directly compares two logically sequential modules and correlates them with a pre-parsed remarks vector
llvm::Expected<AnalysisSession>
PassAnalyzer::runFromModules(std::unique_ptr<llvm::Module> Before,
                              std::unique_ptr<llvm::Module> After,
                              std::vector<Remark>           ExternalRemarks) {
  if (!Before || !After)
    return makeStringError("Null module passed to runFromModules");

  AnalysisSession Session;
  Session.BeforeIR = moduleToString(*Before);
  Session.AfterIR  = moduleToString(*After);
  Session.Remarks  = std::move(ExternalRemarks);
  Session.Diff     = DiffEngine.diff(*Before, *After);
  Session.Diagnostics = DiagEngine.analyze(Session.Remarks, Session.Diff);
  Session.BeforeModule = std::move(Before);
  Session.AfterModule  = std::move(After);
  return Session;
}

// parses an llvm compiler-generated yaml sequence into a structured internal vector of diagnostic remarks
llvm::Expected<std::vector<Remark>>
PassAnalyzer::parseRemarksYAML(llvm::StringRef Path) {
  auto BufOrErr = llvm::MemoryBuffer::getFile(Path);
  if (!BufOrErr)
    return makeStringError("Cannot open remarks file: " + Path);

  std::vector<Remark> Remarks;
  llvm::StringRef Content = (*BufOrErr)->getBuffer();

  auto parseRemarkKind = [](llvm::StringRef K) -> RemarkKind {
    if (K == "Missed")   return RemarkKind::Missed;
    if (K == "Passed")   return RemarkKind::Applied;
    if (K == "Analysis") return RemarkKind::Analysis;
    return RemarkKind::Analysis;
  };

  size_t Pos = 0;
  while (Pos < Content.size()) {
    size_t RecordStart = Content.find("---", Pos);
    if (RecordStart == llvm::StringRef::npos)
      break;

    size_t NextRecord = Content.find("\n---", RecordStart + 3);
    size_t RecordEnd = (NextRecord == llvm::StringRef::npos) ? Content.size() : NextRecord + 1;
    llvm::StringRef Record = Content.slice(RecordStart, RecordEnd);
    Pos = RecordEnd;

    Remark R;
    if (Record.starts_with("--- !Missed"))        R.Kind = RemarkKind::Missed;
    else if (Record.starts_with("--- !Passed"))   R.Kind = RemarkKind::Applied;
    else if (Record.starts_with("--- !Analysis")) R.Kind = RemarkKind::Analysis;
    else continue;

    auto extractField = [&](llvm::StringRef Field) -> std::string {
      size_t FPos = Record.find(Field);
      while (FPos != llvm::StringRef::npos) {
        if (FPos == 0 || Record[FPos - 1] == '\n' || Record[FPos - 1] == ' ' || Record[FPos - 1] == '{')
          break;
        FPos = Record.find(Field, FPos + 1);
      }
      if (FPos == llvm::StringRef::npos) return "";
      size_t LineEnd = Record.find('\n', FPos);
      if (LineEnd == llvm::StringRef::npos) LineEnd = Record.size();
      llvm::StringRef LineStr = Record.slice(FPos + Field.size(), LineEnd);
      size_t QuoteStart = LineStr.find('\'');
      if (QuoteStart == llvm::StringRef::npos) {
        return LineStr.trim().str();
      }
      size_t QuoteEnd = LineStr.find('\'', QuoteStart + 1);
      if (QuoteEnd == llvm::StringRef::npos) return "";
      return LineStr.slice(QuoteStart + 1, QuoteEnd).str();
    };

    R.PassName    = extractField("Pass:");
    R.RemarkName  = extractField("Name:");
    R.FunctionName = extractField("Function:");

    size_t ArgsPos = Record.find("Args:");
    if (ArgsPos != llvm::StringRef::npos) {
      size_t SearchStart = ArgsPos + 5;
      std::string FullMsg;
      while (SearchStart < Record.size()) {
        size_t LineEnd = Record.find('\n', SearchStart);
        if (LineEnd == llvm::StringRef::npos) LineEnd = Record.size();
        llvm::StringRef Line = Record.slice(SearchStart, LineEnd);
        
        if (!Line.trim().starts_with("-") && !Line.trim().empty() && SearchStart > ArgsPos + 6)
          break;

        size_t ValPos = Line.find(": ");
        if (ValPos != llvm::StringRef::npos) {
          llvm::StringRef Val = Line.slice(ValPos + 2, llvm::StringRef::npos).trim();
          std::string Piece;
          if (Val.starts_with("'") && Val.ends_with("'") && Val.size() >= 2) {
            Piece = Val.slice(1, Val.size() - 1).str();
          } else {
            Piece = Val.str();
          }
          if (!FullMsg.empty() && 
              !llvm::StringRef(FullMsg).ends_with(" ") && 
              !Piece.empty() && 
              !llvm::StringRef(Piece).starts_with(" "))
            FullMsg += " ";
          FullMsg += Piece;
        }
        SearchStart = LineEnd + 1;
      }
      R.Message = FullMsg;
    }

    size_t LocPos = Record.find("DebugLoc:");
    if (LocPos != llvm::StringRef::npos) {
      R.Loc.File   = extractField("File:");
      std::string LineStr = extractField("Line:");
      std::string ColStr  = extractField("Column:");
      if (!LineStr.empty()) {
        try { R.Loc.Line = std::stoul(LineStr); } catch (...) {}
      }
      if (!ColStr.empty()) {
        try { R.Loc.Column = std::stoul(ColStr); } catch (...) {}
      }
    }

    if (!R.PassName.empty())
      Remarks.push_back(std::move(R));
  }

  return Remarks;
}

}
