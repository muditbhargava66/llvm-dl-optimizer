#include "Kernels/Activation.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

using namespace llvm;

namespace {

Function *createReLUFunction(Module &M, Type *InputTy, Type *OutputTy) {
  auto *FuncTy = FunctionType::get(Type::getVoidTy(M.getContext()), {InputTy, OutputTy}, false);
  auto *Func = Function::Create(FuncTy, Function::ExternalLinkage, "ReLU", &M);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Func);
  IRBuilder<> Builder(EntryBB);

  auto *Input = Func->getArg(0);
  auto *Output = Func->getArg(1);

  // Get the dimensions of the input tensor
  // Placeholder values
  unsigned InputSize = 1024;

  // Create a loop for the input tensor elements
  BasicBlock *LoopHeader = BasicBlock::Create(M.getContext(), "loop.header", Func);
  BasicBlock *LoopBody = BasicBlock::Create(M.getContext(), "loop.body", Func);
  BasicBlock *LoopExit = BasicBlock::Create(M.getContext(), "loop.exit", Func);

  // Create the loop
  Builder.CreateBr(LoopHeader);
  Builder.SetInsertPoint(LoopHeader);

  // Create the loop index phi node
  auto *IndexPhi = Builder.CreatePHI(Type::getInt64Ty(M.getContext()), 2, "index");
  IndexPhi->addIncoming(ConstantInt::get(Type::getInt64Ty(M.getContext()), 0), EntryBB);

  // Create the loop condition
  auto *Cond = Builder.CreateICmpULT(IndexPhi, ConstantInt::get(Type::getInt64Ty(M.getContext()), InputSize));
  Builder.CreateCondBr(Cond, LoopBody, LoopExit);

  // Create the loop body
  Builder.SetInsertPoint(LoopBody);

  // Load the input value
  auto *InputPtr = Builder.CreateGEP(Input, {Builder.getInt64(0), IndexPhi});
  auto *InputVal = Builder.CreateLoad(InputPtr);

  // Perform the ReLU activation
  auto *Zero = ConstantFP::get(InputVal->getType(), 0.0);
  auto *ReLUVal = Builder.CreateSelect(Builder.CreateFCmpOGT(InputVal, Zero), InputVal, Zero);

  // Store the output value
  auto *OutputPtr = Builder.CreateGEP(Output, {Builder.getInt64(0), IndexPhi});
  Builder.CreateStore(ReLUVal, OutputPtr);

  // Increment the loop index
  auto *NextIndex = Builder.CreateAdd(IndexPhi, ConstantInt::get(Type::getInt64Ty(M.getContext()), 1));
  IndexPhi->addIncoming(NextIndex, LoopBody);
  Builder.CreateBr(LoopHeader);

  // Create the loop exit
  Builder.SetInsertPoint(LoopExit);

  // Create the return instruction
  Builder.CreateRetVoid();

  return Func;
}

} // namespace
