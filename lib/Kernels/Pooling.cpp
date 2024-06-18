#include "Kernels/Pooling.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

using namespace llvm;

namespace {

// Helper function to create a loop
void createLoop(IRBuilder<> &Builder, Value *Start, Value *End, const Twine &Name, std::function<void(IRBuilder<> &, Value *)> Body) {
  Function *Func = Builder.GetInsertBlock()->getParent();
  BasicBlock *PreheaderBB = Builder.GetInsertBlock();
  BasicBlock *LoopBB = BasicBlock::Create(Func->getContext(), Name + ".loop", Func);
  BasicBlock *AfterBB = BasicBlock::Create(Func->getContext(), Name + ".after", Func);

  Builder.CreateCondBr(Builder.CreateICmpULT(Start, End), LoopBB, AfterBB);

  Builder.SetInsertPoint(LoopBB);
  PHINode *Index = Builder.CreatePHI(Start->getType(), 2, Name + ".index");
  Index->addIncoming(Start, PreheaderBB);

  Body(Builder, Index);

  Value *NextVar = Builder.CreateAdd(Index, ConstantInt::get(Start->getType(), 1), Name + ".nextvar");
  Index->addIncoming(NextVar, LoopBB);
  Builder.CreateCondBr(Builder.CreateICmpULT(NextVar, End), LoopBB, AfterBB);

  Builder.SetInsertPoint(AfterBB);
}

Function *createMaxPoolingFunction(Module &M, Type *InputTy, Type *OutputTy,
                                   unsigned KernelH, unsigned KernelW, unsigned StrideH, unsigned StrideW) {
  auto *FuncTy = FunctionType::get(Type::getVoidTy(M.getContext()), {InputTy, OutputTy}, false);
  auto *Func = Function::Create(FuncTy, Function::ExternalLinkage, "maxPooling", &M);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Func);
  IRBuilder<> Builder(EntryBB);

  auto *Input = Func->getArg(0);
  auto *Output = Func->getArg(1);

  // Get the dimensions of the input and output tensors
  // Assume these are defined somewhere appropriately
  // Placeholder values
  unsigned InputH = 32, InputW = 32, OutputH = 30, OutputW = 30;

  // Create loops for the output tensor dimensions
  createLoop(Builder, Builder.getInt32(0), Builder.getInt32(OutputH), "OuterLoopY", [&](IRBuilder<> &Builder, Value *OuterLoopY) {
    createLoop(Builder, Builder.getInt32(0), Builder.getInt32(OutputW), "OuterLoopX", [&](IRBuilder<> &Builder, Value *OuterLoopX) {

      // Initialize the maximum value to a very small number
      auto *OutputIdx = Builder.CreateGEP(Output, {OuterLoopY, OuterLoopX});
      auto *MaxVal = Builder.CreateAlloca(Type::getFloatTy(M.getContext()), nullptr, "maxVal");
      Builder.CreateStore(ConstantFP::get(Type::getFloatTy(M.getContext()), -std::numeric_limits<float>::infinity()), MaxVal);

      // Create loops for the kernel dimensions
      createLoop(Builder, Builder.getInt32(0), Builder.getInt32(KernelH), "InnerLoopY", [&](IRBuilder<> &Builder, Value *InnerLoopY) {
        createLoop(Builder, Builder.getInt32(0), Builder.getInt32(KernelW), "InnerLoopX", [&](IRBuilder<> &Builder, Value *InnerLoopX) {

          // Calculate the input tensor indices based on the current loop indices and strides
          auto *InputIdxY = Builder.CreateAdd(Builder.CreateMul(OuterLoopY, Builder.getInt32(StrideH)), InnerLoopY);
          auto *InputIdxX = Builder.CreateAdd(Builder.CreateMul(OuterLoopX, Builder.getInt32(StrideW)), InnerLoopX);

          // Load the input value
          auto *InputIdx = Builder.CreateGEP(Input, {InputIdxY, InputIdxX});
          auto *InputVal = Builder.CreateLoad(InputIdx);

          // Update the maximum value
          auto *CurrentMax = Builder.CreateLoad(MaxVal);
          auto *NewMax = Builder.CreateSelect(Builder.CreateFCmpOGT(InputVal, CurrentMax), InputVal, CurrentMax);
          Builder.CreateStore(NewMax, MaxVal);

        });
      });

      // Store the maximum value in the output
      auto *FinalMax = Builder.CreateLoad(MaxVal);
      Builder.CreateStore(FinalMax, OutputIdx);

    });
  });

  Builder.CreateRetVoid();

  return Func;
}

} // namespace
