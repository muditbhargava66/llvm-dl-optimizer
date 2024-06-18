#include "Kernels/Pooling.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

using namespace llvm;

namespace {

Function *createMaxPoolingFunction(Module &M, Type *InputTy, Type *OutputTy,
                                   unsigned KernelH, unsigned KernelW, unsigned StrideH, unsigned StrideW) {
  auto *FuncTy = FunctionType::get(Type::getVoidTy(M.getContext()), {InputTy, OutputTy}, false);
  auto *Func = Function::Create(FuncTy, Function::ExternalLinkage, "maxPooling", &M);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Func);
  IRBuilder<> Builder(EntryBB);

  auto *Input = Func->getArg(0);
  auto *Output = Func->getArg(1);

  // Get the dimensions of the input and output tensors
  auto *InputShape = InputTy->getPointerElementType()->getArrayNumElements();
  auto *OutputShape = OutputTy->getPointerElementType()->getArrayNumElements();

  // Create loops for the output tensor dimensions
  auto *OuterLoopY = Builder.CreateLoop(OutputShape[0], "outerLoopY");
  auto *OuterLoopX = Builder.CreateLoop(OutputShape[1], "outerLoopX");
  Builder.SetInsertPoint(OuterLoopX->getBody());

  // Create loops for the kernel dimensions
  auto *InnerLoopY = Builder.CreateLoop(KernelH, "innerLoopY");
  auto *InnerLoopX = Builder.CreateLoop(KernelW, "innerLoopX");
  Builder.SetInsertPoint(InnerLoopX->getBody());

  // Calculate the input tensor indices based on the current loop indices and strides
  auto *InputIdxY = Builder.CreateAdd(Builder.CreateMul(OuterLoopY->getIndVar(), Builder.getInt32(StrideH)), InnerLoopY->getIndVar());
  auto *InputIdxX = Builder.CreateAdd(Builder.CreateMul(OuterLoopX->getIndVar(), Builder.getInt32(StrideW)), InnerLoopX->getIndVar());

  // Load the input value
  auto *InputPtr = Builder.CreateGEP(Input, {Builder.getInt32(0), InputIdxY, InputIdxX});
  auto *InputVal = Builder.CreateLoad(InputPtr);

  // Update the maximum value
  auto *OutputPtr = Builder.CreateGEP(Output, {Builder.getInt32(0), OuterLoopY->getIndVar(), OuterLoopX->getIndVar()});
  auto *OutputVal = Builder.CreateLoad(OutputPtr);
  auto *MaxVal = Builder.CreateSelect(Builder.CreateFCmpOGT(InputVal, OutputVal), InputVal, OutputVal);
  Builder.CreateStore(MaxVal, OutputPtr);

  // Create the return instruction
  Builder.CreateRetVoid();

  return Func;
}

} // end anonymous namespace

namespace llvm {
Function *createMaxPoolingFunction(Module &M, Type *InputTy, Type *OutputTy,
                                   unsigned KernelH, unsigned KernelW, unsigned StrideH, unsigned StrideW) {
  return ::createMaxPoolingFunction(M, InputTy, OutputTy, KernelH, KernelW, StrideH, StrideW);
}
} // namespace llvm