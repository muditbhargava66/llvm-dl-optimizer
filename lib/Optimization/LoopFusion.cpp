#include "Optimization/LoopFusion.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;

namespace {

struct LoopFusionPass : public FunctionPass {
  static char ID;
  LoopFusionPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    if (F.isDeclaration())
      return false;

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    bool Changed = false;

    for (auto &L : LI) {
      if (L->empty() || L->getSubLoops().size() != 1)
        continue;

      Loop *InnerLoop = L->getSubLoops()[0];
      if (!isFusionCandidate(L, InnerLoop))
        continue;

      fuseLoops(L, InnerLoop);
      Changed = true;
    }

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
  }

private:
  bool isFusionCandidate(Loop *OuterLoop, Loop *InnerLoop) {
    // Check if the loops have a single exit and no memory dependencies
    if (!OuterLoop->hasDedicatedExits() || !InnerLoop->hasDedicatedExits())
      return false;

    // Check if the inner loop has no memory dependencies with the outer loop
    for (auto *BB : InnerLoop->getBlocks()) {
      for (auto &I : *BB) {
        if (auto *Inst = dyn_cast<Instruction>(&I)) {
          if (Inst->mayReadOrWriteMemory() &&
              !OuterLoop->isLoopInvariant(Inst->getOperand(0)))
            return false;
        }
      }
    }

    return true;
  }

  void fuseLoops(Loop *OuterLoop, Loop *InnerLoop) {
    // Move the inner loop's basic blocks to the outer loop's exit block
    BasicBlock *OuterLoopExitBlock = OuterLoop->getExitBlock();
    if (OuterLoopExitBlock) {
      for (auto *BB : InnerLoop->blocks()) {
        BB->moveBefore(OuterLoopExitBlock);
      }
    }

    // Update the loop info after moving the basic blocks
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    LI.erase(InnerLoop);
  }
};

} // end anonymous namespace

char LoopFusionPass::ID = 0;
static RegisterPass<LoopFusionPass> X("loop-fusion", "Loop Fusion Pass",
                                      false /* Only looks at CFG */,
                                      false /* Analysis Pass */);

namespace llvm {
FunctionPass *createLoopFusionPass() { return new LoopFusionPass(); }
} // namespace llvm
