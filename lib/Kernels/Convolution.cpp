#include "Kernels/Convolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

using namespace llvm;

namespace {

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
  auto *InputShape = InputTy->getPointerElementType()->getArrayNumElements();
  auto *WeightShape = WeightTy->getPointerElementType()->getArrayNumElements();
  auto *OutputShape = OutputTy->getPointerElementType()->getArrayNumElements();

  // Create loops for the output tensor dimensions
  auto *OuterLoopY = Builder.CreateLoop(OutputShape[0], "outerLoopY");
  auto *OuterLoopX = Builder.CreateLoop(OutputShape[1], "outerLoopX");
  Builder.SetInsertPoint(OuterLoopX->getBody());

  // Create loops for the weight tensor dimensions
  auto *InnerLoopY = Builder.CreateLoop(WeightShape[0], "innerLoopY");
  auto *InnerLoopX = Builder.CreateLoop(WeightShape[1], "innerLoopX");
  Builder.SetInsertPoint(InnerLoopX->getBody());

  // Calculate the input tensor indices based on the current loop indices and strides
  auto *InputIdxY = Builder.CreateAdd(Builder.CreateMul(OuterLoopY->getIndVar(), Builder.getInt32(StrideH)), InnerLoopY->getIndVar());
  auto *InputIdxX = Builder.CreateAdd(Builder.CreateMul(OuterLoopX->getIndVar(), Builder.getInt32(StrideW)), InnerLoopX->getIndVar());

  // Load the input and weight values
  auto *InputPtr = Builder.CreateGEP(Input, {Builder.getInt32(0), InputIdxY, InputIdxX});
  auto *InputVal = Builder.CreateLoad(InputPtr);

  auto *WeightPtr = Builder.CreateGEP(Weight, {Builder.getInt32(0), InnerLoopY->getIndVar(), InnerLoopX->getIndVar()});
  auto *WeightVal = Builder.CreateLoad(WeightPtr);

  // Perform the convolution operation (multiply-accumulate)
  auto *OutputPtr = Builder.CreateGEP(Output, {Builder.getInt32(0), OuterLoopY->getIndVar(), OuterLoopX->getIndVar()});
  auto *OutputVal = Builder.CreateLoad(OutputPtr);
  auto *MulVal = Builder.CreateFMul(InputVal, WeightVal);
  auto *AccumVal = Builder.CreateFAdd(OutputVal, MulVal);
  Builder.CreateStore(AccumVal, OutputPtr);

  // Create the return instruction
  Builder.CreateRetVoid();

  return Func;
}

} // end anonymous namespace

namespace llvm {
Function *createConvolutionFunction(Module &M, Type *InputTy, Type *WeightTy, Type *OutputTy,
                                    unsigned StrideH, unsigned StrideW, unsigned PadH, unsigned PadW) {
  return ::createConvolutionFunction(M, InputTy, WeightTy, OutputTy, StrideH, StrideW, PadH, PadW);
}
} // namespace llvm