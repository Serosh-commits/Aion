#include "OptDebugger/IRDiff.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/WithColor.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <unordered_map>

namespace optdbg {

// prints a value to a string, handling null safely
static std::string printValueToString(const llvm::Value *V) {
  if (!V)
    return "<null>";
  std::string S;
  llvm::raw_string_ostream OS(S);
  V->printAsOperand(OS, false);
  OS.flush();
  return S;
}

// extracts and formats the string representation of an llvm instruction
std::string IRDiffEngine::getInstructionText(const llvm::Instruction &I) {
  std::string S;
  llvm::raw_string_ostream OS(S);
  I.print(OS);
  OS.flush();
  std::string Result = S;
  size_t Pos = Result.find_first_not_of(' ');
  if (Pos != std::string::npos)
    Result = Result.substr(Pos);
  return Result;
}

// resolves a basic block name or generates a synthetic one if unnamed
std::string IRDiffEngine::getBlockName(const llvm::BasicBlock &BB) {
  if (BB.hasName())
    return BB.getName().str();

  const llvm::Function *F = BB.getParent();
  if (!F)
    return "<unnamed>";

  unsigned Idx = 0;
  for (const llvm::BasicBlock &B : *F) {
    if (&B == &BB)
      return "<bb." + std::to_string(Idx) + ">";
    ++Idx;
  }
  return "<unknown>";
}

// stringifies a function's full signature without the body
std::string IRDiffEngine::getFunctionSignature(const llvm::Function &F) {
  std::string S;
  llvm::raw_string_ostream OS(S);
  F.getFunctionType()->print(OS);
  OS.flush();
  return F.getName().str() + " : " + S;
}

// compares the intrinsic llvm attributes, linkage, and visibility of two functions
bool IRDiffEngine::attributesEqual(const llvm::Function &A,
                                    const llvm::Function &B) {
  if (A.getCallingConv() != B.getCallingConv())
    return false;
  if (A.getLinkage() != B.getLinkage())
    return false;
  if (A.getVisibility() != B.getVisibility())
    return false;

  llvm::AttributeList AttrsA = A.getAttributes();
  llvm::AttributeList AttrsB = B.getAttributes();

  std::string SA, SB;
  llvm::raw_string_ostream OSA(SA), OSB(SB);
  AttrsA.print(OSA);
  AttrsB.print(OSB);
  OSA.flush();
  OSB.flush();
  return SA == SB;
}

// caches an llvm instruction's text, opcode, and debug location into a lightweight record
InstructionRecord IRDiffEngine::recordInstruction(const llvm::Instruction &I,
                                                    unsigned LineHint) {
  InstructionRecord Rec;
  Rec.Text       = getInstructionText(I);
  Rec.Line       = LineHint;
  Rec.OpcodeName = I.getOpcodeName();

  if (const llvm::DebugLoc &DL = I.getDebugLoc()) {
    if (DL.get()) {
      Rec.DebugLocStr = DL->getFilename().str() + ":" +
                        std::to_string(DL.getLine()) + ":" +
                        std::to_string(DL.getCol());
    }
  }
  return Rec;
}

// optimal dynamic programming implementation to find the longest common subsequence of two string arrays
std::vector<size_t> IRDiffEngine::computeLCS(
    const std::vector<std::string> &A,
    const std::vector<std::string> &B) {

  size_t M = A.size();
  size_t N = B.size();

  if (M == 0 || N == 0)
    return {};

  std::vector<size_t> DP((M + 1) * (N + 1), 0);
  auto getDP = [&](size_t I, size_t J) -> size_t & {
    return DP[I * (N + 1) + J];
  };

  for (size_t I = 1; I <= M; ++I) {
    for (size_t J = 1; J <= N; ++J) {
      if (A[I - 1] == B[J - 1])
        getDP(I, J) = getDP(I - 1, J - 1) + 1;
      else
        getDP(I, J) = std::max(getDP(I - 1, J), getDP(I, J - 1));
    }
  }

  std::vector<size_t> LCSIndicesInA;
  size_t I = M, J = N;
  while (I > 0 && J > 0) {
    if (A[I - 1] == B[J - 1]) {
      LCSIndicesInA.push_back(I - 1);
      --I;
      --J;
    } else if (getDP(I - 1, J) > getDP(I, J - 1)) {
      --I;
    } else {
      --J;
    }
  }

  std::reverse(LCSIndicesInA.begin(), LCSIndicesInA.end());
  return LCSIndicesInA;
}

// aligns two string arrays using a needleman-wunsch style edit distance matrix
std::vector<std::pair<int, int>> IRDiffEngine::alignSequences(
    const std::vector<std::string> &A,
    const std::vector<std::string> &B) {

  size_t M = A.size();
  size_t N = B.size();

  std::vector<int> DP((M + 1) * (N + 1), 0);
  auto getDP = [&](size_t I, size_t J) -> int & {
    return DP[I * (N + 1) + J];
  };

  for (size_t I = 1; I <= M; ++I)
    for (size_t J = 1; J <= N; ++J)
      if (A[I - 1] == B[J - 1])
        getDP(I, J) = getDP(I - 1, J - 1) + 1;
      else
        getDP(I, J) = std::max(getDP(I - 1, J), getDP(I, J - 1));

  std::vector<std::pair<int, int>> Alignment;
  int I = static_cast<int>(M);
  int J = static_cast<int>(N);

  while (I > 0 || J > 0) {
    if (I > 0 && J > 0 && A[I - 1] == B[J - 1]) {
      Alignment.emplace_back(I - 1, J - 1);
      --I; --J;
    } else if (J > 0 && (I == 0 || getDP(I, J - 1) >= getDP(I - 1, J))) {
      Alignment.emplace_back(-1, J - 1);
      --J;
    } else {
      Alignment.emplace_back(I - 1, -1);
      --I;
    }
  }

  std::reverse(Alignment.begin(), Alignment.end());
  return Alignment;
}

// correlates instructions between two basic blocks and maps them into unedited, added, or removed items
std::vector<InstructionDiff>
IRDiffEngine::diffInstructions(const llvm::BasicBlock &Before,
                                const llvm::BasicBlock &After) {
  std::vector<InstructionRecord> BeforeRecs, AfterRecs;
  std::vector<std::string>       BeforeTexts, AfterTexts;

  unsigned LineIdx = 0;
  for (const llvm::Instruction &I : Before) {
    BeforeRecs.push_back(recordInstruction(I, ++LineIdx));
    BeforeTexts.push_back(BeforeRecs.back().Text);
  }

  LineIdx = 0;
  for (const llvm::Instruction &I : After) {
    AfterRecs.push_back(recordInstruction(I, ++LineIdx));
    AfterTexts.push_back(AfterRecs.back().Text);
  }

  auto Alignment = alignSequences(BeforeTexts, AfterTexts);

  std::vector<InstructionDiff> Result;
  Result.reserve(Alignment.size());

  for (auto [BI, AI] : Alignment) {
    InstructionDiff D;
    if (BI >= 0 && AI >= 0) {
      D.Kind   = DiffKind::Unchanged;
      D.Before = BeforeRecs[BI];
      D.After  = AfterRecs[AI];
    } else if (BI >= 0) {
      D.Kind   = DiffKind::Removed;
      D.Before = BeforeRecs[BI];
    } else {
      assert(AI >= 0);
      D.Kind  = DiffKind::Added;
      D.After = AfterRecs[AI];
    }
    Result.push_back(std::move(D));
  }

  return Result;
}

// performs a structural diff of entire basic blocks by analyzing their inner instruction sequences
BlockDiff IRDiffEngine::diffBlocks(const llvm::BasicBlock &Before,
                                    const llvm::BasicBlock &After,
                                    const std::string      &Name) {
  BlockDiff BD;
  BD.Kind              = DiffKind::Unchanged;
  BD.BlockName         = Name;
  BD.BeforeInstrCount  = Before.size();
  BD.AfterInstrCount   = After.size();
  BD.Instructions      = diffInstructions(Before, After);

  bool AnyChange = false;
  for (const auto &ID : BD.Instructions) {
    if (ID.Kind != DiffKind::Unchanged) {
      AnyChange = true;
      break;
    }
  }

  if (AnyChange)
    BD.Kind = DiffKind::Modified;

  return BD;
}

// orchestrates block-level differencing mapping for the complete architecture of a given function
FunctionDiff IRDiffEngine::diffFunctions(const llvm::Function &Before,
                                          const llvm::Function &After) {
  FunctionDiff FD;
  FD.FunctionName      = Before.getName().str();
  FD.BeforeSignature   = getFunctionSignature(Before);
  FD.AfterSignature    = getFunctionSignature(After);
  FD.BeforeBlockCount  = Before.size();
  FD.AfterBlockCount   = After.size();
  FD.AttributesChanged = !attributesEqual(Before, After);
  FD.SignatureChanged  = FD.BeforeSignature != FD.AfterSignature;

  FD.BeforeInstrCount = 0;
  for (const llvm::BasicBlock &BB : Before)
    FD.BeforeInstrCount += BB.size();

  FD.AfterInstrCount = 0;
  for (const llvm::BasicBlock &BB : After)
    FD.AfterInstrCount += BB.size();

  if (Before.isDeclaration() && After.isDeclaration()) {
    FD.Kind = DiffKind::Unchanged;
    return FD;
  }

  if (Before.isDeclaration() || After.isDeclaration()) {
    FD.Kind = DiffKind::Modified;
    return FD;
  }

  std::vector<std::string> BeforeNames, AfterNames;
  std::vector<const llvm::BasicBlock*> BeforeBlocksV, AfterBlocksV;
  
  unsigned Idx = 0;
  for (const llvm::BasicBlock &BB : Before) {
    BeforeNames.push_back(BB.hasName() ? BB.getName().str() : "<bb." + std::to_string(Idx) + ">");
    BeforeBlocksV.push_back(&BB);
    ++Idx;
  }
  
  Idx = 0;
  for (const llvm::BasicBlock &BB : After) {
    AfterNames.push_back(BB.hasName() ? BB.getName().str() : "<bb." + std::to_string(Idx) + ">");
    AfterBlocksV.push_back(&BB);
    ++Idx;
  }


  auto BlockAlignment = alignSequences(BeforeNames, AfterNames);

  bool AnyChange = false;

  for (auto [BI, AI] : BlockAlignment) {
    if (BI >= 0 && AI >= 0) {
      const llvm::BasicBlock *BBefore = BeforeBlocksV[BI];
      const llvm::BasicBlock *BAfter  = AfterBlocksV[AI];
      BlockDiff BD = diffBlocks(*BBefore, *BAfter, BeforeNames[BI]);
      if (BD.Kind != DiffKind::Unchanged)
        AnyChange = true;
      FD.Blocks.push_back(std::move(BD));
    } else if (BI >= 0) {
      const llvm::BasicBlock *BBefore = BeforeBlocksV[BI];
      BlockDiff BD;
      BD.Kind              = DiffKind::Removed;
      BD.BlockName         = getBlockName(*BBefore);
      BD.BeforeInstrCount  = BBefore->size();
      BD.AfterInstrCount   = 0;
      AnyChange = true;
      FD.Blocks.push_back(std::move(BD));
    } else {
      assert(AI >= 0);
      const llvm::BasicBlock *BAfter = AfterBlocksV[AI];
      BlockDiff BD;
      BD.Kind              = DiffKind::Added;
      BD.BlockName         = getBlockName(*BAfter);
      BD.BeforeInstrCount  = 0;
      BD.AfterInstrCount   = BAfter->size();
      AnyChange = true;
      FD.Blocks.push_back(std::move(BD));
    }
  }

  if (!AnyChange && !FD.AttributesChanged && !FD.SignatureChanged)
    FD.Kind = DiffKind::Unchanged;
  else
    FD.Kind = DiffKind::Modified;

  return FD;
}

// computes a comprehensive difference report comparing two llvm modules by mapping and analyzing all internal functions
ModuleDiff IRDiffEngine::diff(const llvm::Module &Before,
                               const llvm::Module &After) {
  ModuleDiff MD;
  MD.AddedFunctions     = 0;
  MD.RemovedFunctions   = 0;
  MD.ModifiedFunctions  = 0;
  MD.UnchangedFunctions = 0;
  MD.TotalBeforeInstructions = 0;
  MD.TotalAfterInstructions  = 0;

  std::unordered_map<std::string, const llvm::Function *> BeforeFuncs;
  for (const llvm::Function &F : Before) {
    BeforeFuncs[F.getName().str()] = &F;
    for (const llvm::BasicBlock &BB : F)
      MD.TotalBeforeInstructions += BB.size();
  }

  std::unordered_map<std::string, const llvm::Function *> AfterFuncs;
  for (const llvm::Function &F : After) {
    AfterFuncs[F.getName().str()] = &F;
    for (const llvm::BasicBlock &BB : F)
      MD.TotalAfterInstructions += BB.size();
  }

  for (const auto &[Name, FBefore] : BeforeFuncs) {
    auto It = AfterFuncs.find(Name);
    if (It == AfterFuncs.end()) {
      FunctionDiff FD;
      FD.Kind          = DiffKind::Removed;
      FD.FunctionName  = Name;
      FD.BeforeSignature = getFunctionSignature(*FBefore);
      FD.BeforeBlockCount = FBefore->size();
      FD.BeforeInstrCount = 0;
      for (const llvm::BasicBlock &BB : *FBefore)
        FD.BeforeInstrCount += BB.size();
      FD.AfterBlockCount  = 0;
      FD.AfterInstrCount  = 0;
      FD.AttributesChanged = false;
      FD.SignatureChanged  = false;
      MD.Functions.push_back(std::move(FD));
      ++MD.RemovedFunctions;
    } else {
      FunctionDiff FD = diffFunctions(*FBefore, *It->second);
      if (FD.Kind == DiffKind::Modified || FD.AttributesChanged || FD.SignatureChanged)
        ++MD.ModifiedFunctions;
      else
        ++MD.UnchangedFunctions;
      MD.Functions.push_back(std::move(FD));
    }
  }

  for (const auto &[Name, FAfter] : AfterFuncs) {
    if (BeforeFuncs.find(Name) == BeforeFuncs.end()) {
      FunctionDiff FD;
      FD.Kind          = DiffKind::Added;
      FD.FunctionName  = Name;
      FD.AfterSignature = getFunctionSignature(*FAfter);
      FD.BeforeBlockCount = 0;
      FD.BeforeInstrCount = 0;
      FD.AfterBlockCount  = FAfter->size();
      FD.AfterInstrCount  = 0;
      for (const llvm::BasicBlock &BB : *FAfter)
        FD.AfterInstrCount += BB.size();
      FD.AttributesChanged = false;
      FD.SignatureChanged  = false;
      MD.Functions.push_back(std::move(FD));
      ++MD.AddedFunctions;
    }
  }

  return MD;
}

// checks if function instruction count decreased, indicating general optimization
bool FunctionDiff::wasOptimized() const {
  return Kind == DiffKind::Modified && AfterInstrCount < BeforeInstrCount;
}

// checks if function basic block count decreased, indicating structural simplification
bool FunctionDiff::wasSimplified() const {
  return Kind == DiffKind::Modified && AfterBlockCount < BeforeBlockCount;
}

// checks if the function was completely removed, implying it was inlined or dead-code eliminated
bool FunctionDiff::wasInlined() const {
  return Kind == DiffKind::Removed;
}

// formats and prints the generated module diff structural report to an output stream
void printModuleDiff(const ModuleDiff &Diff, llvm::raw_ostream &OS,
                     bool UseColor) {
  auto ColorFor = [&](DiffKind K) -> llvm::raw_ostream::Colors {
    switch (K) {
    case DiffKind::Added:     return llvm::raw_ostream::GREEN;
    case DiffKind::Removed:   return llvm::raw_ostream::RED;
    case DiffKind::Modified:  return llvm::raw_ostream::YELLOW;
    case DiffKind::Unchanged: return llvm::raw_ostream::WHITE;
    }
    return llvm::raw_ostream::WHITE;
  };

  OS << "\n=== IR Diff ===\n";
  OS << "Functions: +" << Diff.AddedFunctions
     << " -" << Diff.RemovedFunctions
     << " ~" << Diff.ModifiedFunctions
     << " =" << Diff.UnchangedFunctions << "\n";
  OS << "Instructions: " << Diff.TotalBeforeInstructions
     << " -> " << Diff.TotalAfterInstructions;
  int64_t Delta = Diff.instructionDelta();
  if (Delta < 0)
    OS << " (" << Delta << ")\n";
  else if (Delta > 0)
    OS << " (+" << Delta << ")\n";
  else
    OS << " (no change)\n";

  for (const FunctionDiff &FD : Diff.Functions) {
    if (FD.Kind == DiffKind::Unchanged)
      continue;

    if (UseColor)
      OS.changeColor(ColorFor(FD.Kind));

    switch (FD.Kind) {
    case DiffKind::Added:
      OS << "[+] @" << FD.FunctionName << " (new function)\n";
      break;
    case DiffKind::Removed:
      OS << "[-] @" << FD.FunctionName << " (inlined/removed)\n";
      break;
    case DiffKind::Modified:
      OS << "[~] @" << FD.FunctionName
         << "  blocks: " << FD.BeforeBlockCount << " -> " << FD.AfterBlockCount
         << "  instrs: " << FD.BeforeInstrCount << " -> " << FD.AfterInstrCount
         << "\n";
      break;
    case DiffKind::Unchanged:
      break;
    }

    if (UseColor)
      OS.resetColor();

    for (const BlockDiff &BD : FD.Blocks) {
      if (BD.Kind == DiffKind::Unchanged)
        continue;

      OS << "  ";
      if (UseColor)
        OS.changeColor(ColorFor(BD.Kind));

      switch (BD.Kind) {
      case DiffKind::Added:
        OS << "[+] %" << BD.BlockName << ":\n";
        break;
      case DiffKind::Removed:
        OS << "[-] %" << BD.BlockName << ":\n";
        break;
      case DiffKind::Modified:
        OS << "[~] %" << BD.BlockName << ":\n";
        break;
      case DiffKind::Unchanged:
        break;
      }

      if (UseColor)
        OS.resetColor();

      for (const InstructionDiff &ID : BD.Instructions) {
        if (ID.Kind == DiffKind::Unchanged)
          continue;
        OS << "    ";
        if (UseColor)
          OS.changeColor(ColorFor(ID.Kind));
        switch (ID.Kind) {
        case DiffKind::Added:
          OS << "+ " << ID.After.Text << "\n";
          break;
        case DiffKind::Removed:
          OS << "- " << ID.Before.Text << "\n";
          break;
        case DiffKind::Modified:
          OS << "- " << ID.Before.Text << "\n";
          OS << "    + " << ID.After.Text << "\n";
          break;
        case DiffKind::Unchanged:
          break;
        }
        if (UseColor)
          OS.resetColor();
      }
    }
  }
}

}
