#pragma once

#include "OptDebugger/Support.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <string>
#include <vector>

namespace optdbg {

enum class DiffKind : uint8_t {
  Unchanged,
  Added,
  Removed,
  Modified,
};

struct InstructionRecord {
  std::string Text;
  unsigned    Line;
  std::string OpcodeName;
  std::string DebugLocStr;
};

struct InstructionDiff {
  DiffKind              Kind;
  InstructionRecord     Before;
  InstructionRecord     After;
};

struct BlockDiff {
  DiffKind                        Kind;
  std::string                     BlockName;
  std::vector<InstructionDiff>    Instructions;
  size_t                          BeforeInstrCount;
  size_t                          AfterInstrCount;
};

struct FunctionDiff {
  DiffKind                    Kind;
  std::string                 FunctionName;
  std::string                 BeforeSignature;
  std::string                 AfterSignature;
  std::vector<BlockDiff>      Blocks;
  size_t                      BeforeBlockCount;
  size_t                      AfterBlockCount;
  size_t                      BeforeInstrCount;
  size_t                      AfterInstrCount;
  bool                        AttributesChanged;
  bool                        SignatureChanged;

  bool wasOptimized() const;
  bool wasSimplified() const;
  bool wasInlined() const;
};

struct ModuleDiff {
  std::vector<FunctionDiff>   Functions;
  size_t                      AddedFunctions;
  size_t                      RemovedFunctions;
  size_t                      ModifiedFunctions;
  size_t                      UnchangedFunctions;
  size_t                      TotalBeforeInstructions;
  size_t                      TotalAfterInstructions;

  bool hasChanges() const { return ModifiedFunctions > 0 || AddedFunctions > 0 || RemovedFunctions > 0; }
  int64_t instructionDelta() const {
    return static_cast<int64_t>(TotalAfterInstructions) -
           static_cast<int64_t>(TotalBeforeInstructions);
  }
};

class IRDiffEngine {
public:
  IRDiffEngine()  = default;
  ~IRDiffEngine() = default;

  IRDiffEngine(const IRDiffEngine &)            = delete;
  IRDiffEngine &operator=(const IRDiffEngine &) = delete;

  ModuleDiff diff(const llvm::Module &Before, const llvm::Module &After);

private:
  FunctionDiff diffFunctions(const llvm::Function &Before,
                              const llvm::Function &After);

  BlockDiff diffBlocks(const llvm::BasicBlock &Before,
                        const llvm::BasicBlock &After,
                        const std::string      &Name);

  std::vector<InstructionDiff>
  diffInstructions(const llvm::BasicBlock &Before,
                    const llvm::BasicBlock &After);

  InstructionRecord recordInstruction(const llvm::Instruction &I,
                                       unsigned LineHint);

  static std::string getInstructionText(const llvm::Instruction &I);
  static std::string getBlockName(const llvm::BasicBlock &BB);
  static std::string getFunctionSignature(const llvm::Function &F);
  static bool attributesEqual(const llvm::Function &A,
                               const llvm::Function &B);

  std::vector<size_t> computeLCS(
      const std::vector<std::string> &A,
      const std::vector<std::string> &B);

  std::vector<std::pair<int, int>> alignSequences(
      const std::vector<std::string> &A,
      const std::vector<std::string> &B);
};

void printModuleDiff(const ModuleDiff &Diff, llvm::raw_ostream &OS,
                     bool UseColor);

}
