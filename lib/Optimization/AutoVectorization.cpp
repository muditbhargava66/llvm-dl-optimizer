#include "Optimization/AutoVectorization.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Vectorize/LoopVectorize.h"

using namespace llvm;

namespace {

struct AutoVectorizationPass : public FunctionPass {
  static char ID;
  AutoVectorizationPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    if (F.isDeclaration())
      return false;

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    TargetTransformInfo &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    bool Changed = false;

    for (auto *Loop : LI) {
      if (!Loop->isInnermost() || !Loop->isLoopSimplifyForm())
        continue;

      if (canVectorizeLoop(Loop, SE, TTI, DT)) {
        vectorizeLoop(Loop, SE, TTI, DT);
        Changed = true;
      }
    }

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
  }

private:
  bool canVectorizeLoop(Loop *L, ScalarEvolution &SE, TargetTransformInfo &TTI, DominatorTree &DT) {
    // Check if the loop has a single block and a single backedge
    if (!L->getExitingBlock() || !L->getLoopLatch())
      return false;

    // Check if the loop has a constant trip count
    const SCEV *TripCount = SE.getBackedgeTakenCount(L);
    if (isa<SCEVCouldNotCompute>(TripCount))
      return false;

    // Check if the loop is supported by the target's vectorizer
    if (!TTI.isLegalToVectorize(L))
      return false;

    // Check if the loop has no memory dependencies
    for (auto *BB : L->getBlocks()) {
      for (auto &I : *BB) {
        if (auto *Inst = dyn_cast<Instruction>(&I)) {
          if (Inst->mayReadOrWriteMemory() && !L->isLoopInvariant(Inst->getOperand(0)))
            return false;
        }
      }
    }

    return true;
  }

  void vectorizeLoop(Loop *L, ScalarEvolution &SE, TargetTransformInfo &TTI, DominatorTree &DT) {
    LoopVectorizePass LVP;
    LVP.run(*L, SE, TTI, DT);
  }
};

} // end anonymous namespace

char AutoVectorizationPass::ID = 0;
static RegisterPass<AutoVectorizationPass> X("auto-vectorization", "Auto Vectorization Pass",
                                             false /* Only looks at CFG */,
                                             false /* Analysis Pass */);

namespace llvm {
FunctionPass *createAutoVectorizationPass() { return new AutoVectorizationPass(); }
} // namespace llvm