#pragma once

#include "OptDebugger/Support.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DiagnosticHandler.h"
#include <mutex>
#include <vector>

namespace optdbg {

class RemarkCollectorHandler : public llvm::DiagnosticHandler {
public:
  RemarkCollectorHandler(std::vector<Remark> &Remarks) : CollectedRemarks(Remarks) {}

  bool handleDiagnostics(const llvm::DiagnosticInfo &DI) override;

private:
  std::vector<Remark> &CollectedRemarks;
  std::mutex Mutex;

  static SourceLocation convertLocation(const llvm::DiagnosticLocation &Loc);
  static std::string buildMessage(const llvm::DiagnosticInfoOptimizationBase &DI);
  Remark convertRemark(const llvm::DiagnosticInfoOptimizationBase &DI);
};

class RemarkCollector {
public:
  RemarkCollector() = default;
  void install(llvm::LLVMContext &Ctx);
  const std::vector<Remark> &getRemarks() const { return Remarks; }

  std::vector<Remark> getMissedRemarks() const;
  std::vector<Remark> getAppliedRemarks() const;
  std::vector<Remark> getAnalysisRemarks() const;
  std::vector<Remark> getRemarksForFunction(llvm::StringRef FunctionName) const;
  std::vector<Remark> getRemarksForPass(llvm::StringRef PassName) const;

private:
  std::vector<Remark> Remarks;
};

}
