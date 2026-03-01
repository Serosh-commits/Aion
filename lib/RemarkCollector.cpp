#include "OptDebugger/RemarkCollector.h"

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>

namespace optdbg {

// converts llvm diagnostic location to our internal sourcelocation format
SourceLocation
RemarkCollectorHandler::convertLocation(const llvm::DiagnosticLocation &Loc) {
  SourceLocation SL;
  if (Loc.isValid()) {
    SL.File   = Loc.getRelativePath().str();
    SL.Line   = Loc.getLine();
    SL.Column = Loc.getColumn();
  }
  return SL;
}

// extracts the raw diagnostic message string from llvm's diagnostic printer
std::string RemarkCollectorHandler::buildMessage(
    const llvm::DiagnosticInfoOptimizationBase &DI) {
  std::string Msg;
  llvm::raw_string_ostream OS(Msg);
  llvm::DiagnosticPrinterRawOStream DP(OS);
  DI.print(DP);
  OS.flush();

  size_t Pos = Msg.find(':');
  if (Pos != std::string::npos) {
    Pos = Msg.find_first_not_of(' ', Pos + 1);
    if (Pos != std::string::npos)
      return Msg.substr(Pos);
  }
  return Msg;
}

// translates an llvm optimization diagnostic into a unified remark datastructure
Remark RemarkCollectorHandler::convertRemark(
    const llvm::DiagnosticInfoOptimizationBase &DI) {
  Remark R;

  unsigned Kind = DI.getKind();
  bool IsMachine = (Kind >= llvm::DK_MachineOptimizationRemark &&
                    Kind <= llvm::DK_MachineOptimizationRemarkAnalysis);

  if (Kind == llvm::DK_OptimizationRemark ||
      Kind == llvm::DK_MachineOptimizationRemark)
    R.Kind = RemarkKind::Applied;
  else if (Kind == llvm::DK_OptimizationRemarkMissed ||
           Kind == llvm::DK_MachineOptimizationRemarkMissed)
    R.Kind = RemarkKind::Missed;
  else if (Kind == llvm::DK_OptimizationRemarkAnalysis ||
           Kind == llvm::DK_MachineOptimizationRemarkAnalysis)
    R.Kind = RemarkKind::Analysis;
  else if (Kind == llvm::DK_OptimizationRemarkAnalysisAliasing)
    R.Kind = RemarkKind::AnalysisAliasing;
  else if (Kind == llvm::DK_OptimizationRemarkAnalysisFPCommute)
    R.Kind = RemarkKind::AnalysisFPCommute;
  else
    R.Kind = RemarkKind::Analysis;

  R.IsMachine = IsMachine;

  R.PassName    = DI.getPassName().str();
  R.RemarkName  = DI.getRemarkName().str();
  R.FunctionName = DI.getFunction().getName().str();
  R.Loc          = convertLocation(DI.getLocation());
  R.Message      = buildMessage(DI);

  for (const llvm::DiagnosticInfoOptimizationBase::Argument &Arg : DI.getArgs()) {
    RemarkArgument RA;
    RA.Key   = Arg.Key;
    RA.Value = Arg.Val;
    if (Arg.Loc.isValid())
      RA.Loc = convertLocation(Arg.Loc);
    R.Args.push_back(std::move(RA));
  }

  if (DI.getHotness())
    R.Hotness = *DI.getHotness();

  return R;
}

// translates an llvm resource limit diagnostic (like stack size) into a unified remark
Remark RemarkCollectorHandler::convertRemark(
    const llvm::DiagnosticInfoResourceLimit &DI) {
  Remark R;
  R.Kind = RemarkKind::Analysis;
  R.IsMachine = true;
  R.PassName = "backend";
  R.RemarkName = DI.getResourceName();
  R.FunctionName = DI.getFunction().getName().str();

  std::string Msg;
  {
    llvm::raw_string_ostream OS(Msg);
    llvm::DiagnosticPrinterRawOStream DP(OS);
    DI.print(DP);
  }
  R.Message = Msg;

  RemarkArgument SizeArg;
  SizeArg.Key = "Size";
  SizeArg.Value = std::to_string(DI.getResourceSize());
  R.Args.push_back(std::move(SizeArg));

  RemarkArgument LimitArg;
  LimitArg.Key = "Limit";
  LimitArg.Value = std::to_string(DI.getResourceLimit());
  R.Args.push_back(std::move(LimitArg));

  return R;
}

// hooks into llvm's diagnostic stream to filter and capture relevant optimization remarks
bool RemarkCollectorHandler::handleDiagnostics(
    const llvm::DiagnosticInfo &DI) {
  unsigned Kind = DI.getKind();

  bool IsOptRemark =
      Kind == llvm::DK_OptimizationRemark ||
      Kind == llvm::DK_OptimizationRemarkMissed ||
      Kind == llvm::DK_OptimizationRemarkAnalysis ||
      Kind == llvm::DK_OptimizationRemarkAnalysisAliasing ||
      Kind == llvm::DK_OptimizationRemarkAnalysisFPCommute ||
      Kind == llvm::DK_MachineOptimizationRemark ||
      Kind == llvm::DK_MachineOptimizationRemarkMissed ||
      Kind == llvm::DK_MachineOptimizationRemarkAnalysis;

  bool IsResourceLimit = (Kind == llvm::DK_ResourceLimit ||
                          Kind == llvm::DK_StackSize);

  if (!IsOptRemark && !IsResourceLimit)
    return false;

  Remark R;
  if (const auto *OptDI = llvm::dyn_cast<llvm::DiagnosticInfoOptimizationBase>(&DI)) {
    R = convertRemark(*OptDI);
  } else if (const auto *ResDI = llvm::dyn_cast<llvm::DiagnosticInfoResourceLimit>(&DI)) {
    R = convertRemark(*ResDI);
  } else {
    return false;
  }

  std::lock_guard<std::mutex> Guard(Mutex);
  CollectedRemarks.push_back(std::move(R));
  return true;
}

// installs the custom diagnostic handler into the llvm context
void RemarkCollector::install(llvm::LLVMContext &Ctx) {
  Ctx.setDiagnosticHandler(
      std::make_unique<RemarkCollectorHandler>(Remarks));
  Ctx.setDiagnosticsHotnessRequested(false);
}

// filters collected remarks to return only missed optimizations
std::vector<Remark> RemarkCollector::getMissedRemarks() const {
  std::vector<Remark> Result;
  for (const Remark &R : Remarks)
    if (R.isMissed())
      Result.push_back(R);
  return Result;
}

// filters collected remarks to return only successful optimizations
std::vector<Remark> RemarkCollector::getAppliedRemarks() const {
  std::vector<Remark> Result;
  for (const Remark &R : Remarks)
    if (R.isApplied())
      Result.push_back(R);
  return Result;
}

// filters collected remarks to return only general analysis notes
std::vector<Remark> RemarkCollector::getAnalysisRemarks() const {
  std::vector<Remark> Result;
  for (const Remark &R : Remarks)
    if (R.isAnalysis())
      Result.push_back(R);
  return Result;
}

// queries the collected remarks for a specific function name
std::vector<Remark>
RemarkCollector::getRemarksForFunction(llvm::StringRef FunctionName) const {
  std::vector<Remark> Result;
  for (const Remark &R : Remarks)
    if (R.FunctionName == FunctionName.str())
      Result.push_back(R);
  return Result;
}

// queries the collected remarks triggered by a specific llvm pass
std::vector<Remark>
RemarkCollector::getRemarksForPass(llvm::StringRef PassName) const {
  std::vector<Remark> Result;
  for (const Remark &R : Remarks)
    if (R.PassName == PassName.str())
      Result.push_back(R);
  return Result;
}

}
