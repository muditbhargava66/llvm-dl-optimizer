#include "Optimization/DataLayoutTransform.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

namespace {

struct DataLayoutTransformPass : public FunctionPass {
  static char ID;
  DataLayoutTransformPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    if (F.isDeclaration())
      return false;

    bool Changed = false;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *Alloca = dyn_cast<AllocaInst>(&I)) {
          Type *AllocatedType = Alloca->getAllocatedType();
          if (isPaddingRequired(AllocatedType)) {
            transformAllocaWithPadding(Alloca);
            Changed = true;
          }
        }
      }
    }

    return Changed;
  }

private:
  bool isPaddingRequired(Type *T) {
    // Check if the type requires padding for better memory alignment
    return T->isSized() && (T->getPrimitiveSizeInBits() % 64 != 0);
  }

  void transformAllocaWithPadding(AllocaInst *Alloca) {
    IRBuilder<> Builder(Alloca);
    Type *AllocatedType = Alloca->getAllocatedType();
    unsigned AllocatedSize = Alloca->getModule()->getDataLayout().getTypeAllocSize(AllocatedType);
    unsigned PaddedSize = alignTo(AllocatedSize, 8); // Align to 64 bits

    // Create a new alloca with the padded size
    Type *PaddedType = ArrayType::get(Builder.getInt8Ty(), PaddedSize);
    AllocaInst *PaddedAlloca = Builder.CreateAlloca(PaddedType, nullptr, Alloca->getName() + ".padded");

    // Replace all uses of the original alloca with the padded alloca
    Alloca->replaceAllUsesWith(Builder.CreateBitCast(PaddedAlloca, Alloca->getType()));
    Alloca->eraseFromParent();
  }
};

} // end anonymous namespace

char DataLayoutTransformPass::ID = 0;
static RegisterPass<DataLayoutTransformPass> X("data-layout-transform", "Data Layout Transformation Pass",
                                               false /* Only looks at CFG */,
                                               false /* Analysis Pass */);

namespace llvm {
FunctionPass *createDataLayoutTransformPass() { return new DataLayoutTransformPass(); }
} // namespace llvm
