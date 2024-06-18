#include "Kernels/Convolution.h"
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

Function *createConvolutionFunction(Module &M, Type *InputTy, Type *WeightTy, Type *OutputTy,
                                    unsigned StrideH, unsigned StrideW, unsigned PadH, unsigned PadW) {
  auto *FuncTy = FunctionType::get(Type::getVoidTy(M.getContext()), {InputTy, WeightTy, OutputTy}, false);
  auto *Func = Function::Create(FuncTy, Function::ExternalLinkage, "convolution", &M);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Func);
  IRBuilder<> Builder(EntryBB);

  auto *Input = Func->getArg(0);
  auto *Weight = Func->getArg(1);
  auto *Output = Func->getArg(2);

  // Get the dimensions of the input, weight, and output tensors
  // Assume these are defined somewhere appropriately
  // Placeholder values
  unsigned InputH = 32, InputW = 32, WeightH = 3, WeightW = 3, OutputH = 30, OutputW = 30;

  // Create loops for the output tensor dimensions
  createLoop(Builder, Builder.getInt32(0), Builder.getInt32(OutputH), "OuterLoopY", [&](IRBuilder<> &Builder, Value *OuterLoopY) {
    createLoop(Builder, Builder.getInt32(0), Builder.getInt32(OutputW), "OuterLoopX", [&](IRBuilder<> &Builder, Value *OuterLoopX) {

      // Initialize the output to zero
      auto *OutputIdx = Builder.CreateGEP(Output, {OuterLoopY, OuterLoopX});
      Builder.CreateStore(ConstantFP::get(Type::getFloatTy(M.getContext()), 0.0), OutputIdx);

      // Create loops for the weight tensor dimensions
      createLoop(Builder, Builder.getInt32(0), Builder.getInt32(WeightH), "InnerLoopY", [&](IRBuilder<> &Builder, Value *InnerLoopY) {
        createLoop(Builder, Builder.getInt32(0), Builder.getInt32(WeightW), "InnerLoopX", [&](IRBuilder<> &Builder, Value *InnerLoopX) {

          // Calculate the input tensor indices based on the current loop indices and strides
          auto *InputIdxY = Builder.CreateAdd(Builder.CreateMul(OuterLoopY, Builder.getInt32(StrideH)), InnerLoopY);
          auto *InputIdxX = Builder.CreateAdd(Builder.CreateMul(OuterLoopX, Builder.getInt32(StrideW)), InnerLoopX);

          // Load the input and weight values
          auto *InputIdx = Builder.CreateGEP(Input, {InputIdxY, InputIdxX});
          auto *WeightIdx = Builder.CreateGEP(Weight, {InnerLoopY, InnerLoopX});
          auto *InputVal = Builder.CreateLoad(InputIdx);
          auto *WeightVal = Builder.CreateLoad(WeightIdx);

          // Perform the multiplication and accumulate the result in the output
          auto *OutputVal = Builder.CreateLoad(OutputIdx);
          auto *MulVal = Builder.CreateFMul(InputVal, WeightVal);
          auto *AccVal = Builder.CreateFAdd(OutputVal, MulVal);
          Builder.CreateStore(AccVal, OutputIdx);

        });
      });

    });
  });

  Builder.CreateRetVoid();

  return Func;
}

} // namespace
