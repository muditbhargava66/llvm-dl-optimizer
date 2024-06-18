// Activation.cpp
#include "Kernels/Activation.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

using namespace llvm;

/**
 * Creates a ReLU activation function.
 *
 * @param M The LLVM module to create the function in.
 * @param InputTy The type of the input tensor.
 * @param OutputTy The type of the output tensor.
 * @param InputShape The shape of the input tensor.
 * @return The created ReLU function.
 */
Function *createReLUFunction(Module &M, Type *InputTy, Type *OutputTy, uint64_t InputShape) {
  // Create the function type
  auto *FuncTy = FunctionType::get(Type::getVoidTy(M.getContext()), {InputTy, OutputTy}, false);

  // Create the function
  auto *Func = Function::Create(FuncTy, Function::ExternalLinkage, "relu", &M);

  // Create the entry basic block
  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Func);

  // Create the IR builder
  IRBuilder<> Builder(EntryBB);

  // Get the function arguments
  auto *Input = Func->getArg(0);
  auto *Output = Func->getArg(1);

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
  auto *Cond = Builder.CreateICmpULT(IndexPhi, ConstantInt::get(Type::getInt64Ty(M.getContext()), InputShape));
  Builder.CreateCondBr(Cond, LoopBody, LoopExit);

  // Create the loop body
  Builder.SetInsertPoint(LoopBody);

  // Load the input value
  Type *InputPtrTy = InputTy->getElementType();
  auto *InputPtr = Builder.CreateGEP(InputPtrTy, Input, {Builder.getInt32(0), IndexPhi});
  auto *InputVal = Builder.CreateLoad(InputPtr);

  // Perform the ReLU activation
  auto *Zero = ConstantFP::get(InputVal->getType(), 0.0);
  auto *ReLUVal = Builder.CreateSelect(Builder.CreateFCmpOGT(InputVal, Zero), InputVal, Zero);

  // Store the output value
  Type *OutputPtrTy = OutputTy->getElementType();
  auto *OutputPtr = Builder.CreateGEP(OutputPtrTy, Output, {Builder.getInt32(0), IndexPhi});
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