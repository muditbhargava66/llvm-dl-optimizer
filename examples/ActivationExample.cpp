#include "Kernels/Activation.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

int main() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  LLVMContext Context;
  Module M("ActivationExample", Context);

  Type *FloatTy = Type::getFloatTy(Context);
  Type *InputTy = PointerType::get(FloatTy, 0);
  Type *OutputTy = PointerType::get(FloatTy, 0);

  Function *ReLUFunc = createReLUFunction(M, InputTy, OutputTy);

  // Create input tensor
  std::vector<float> InputData = {-1.0f, 2.0f, -3.0f, 4.0f, -5.0f, 6.0f};
  Constant *InputTensor = ConstantDataArray::get(Context, InputData);

  // Create output tensor
  std::vector<float> OutputData(6, 0.0f);
  GlobalVariable *OutputTensor = new GlobalVariable(M, ArrayType::get(FloatTy, 6), false,
                                                    GlobalValue::ExternalLinkage, nullptr, "output");
  OutputTensor->setInitializer(ConstantDataArray::get(Context, OutputData));

  // Create the main function
  Function *MainFunc = Function::Create(FunctionType::get(Type::getInt32Ty(Context), false),
                                        GlobalValue::ExternalLinkage, "main", &M);
  BasicBlock *BB = BasicBlock::Create(Context, "entry", MainFunc);
  IRBuilder<> Builder(BB);

  Value *InputPtr = Builder.CreatePointerCast(InputTensor, InputTy);
  Value *OutputPtr = Builder.CreatePointerCast(OutputTensor, OutputTy);
  Builder.CreateCall(ReLUFunc, {InputPtr, OutputPtr});

  // Print the output tensor
  for (unsigned i = 0; i < 6; ++i) {
    Value *OutputElemPtr = Builder.CreateConstGEP1_32(OutputPtr, i);
    Value *OutputElem = Builder.CreateLoad(OutputElemPtr);
    Builder.CreateCall(M.getOrInsertFunction("printf", FunctionType::get(Type::getInt32Ty(Context), {Type::getInt8PtrTy(Context)}, true)),
                       {Builder.CreateGlobalStringPtr("%f "), OutputElem});
  }
  Builder.CreateCall(M.getOrInsertFunction("printf", FunctionType::get(Type::getInt32Ty(Context), {Type::getInt8PtrTy(Context)}, true)),
                     {Builder.CreateGlobalStringPtr("\n")});

  Builder.CreateRet(ConstantInt::get(Type::getInt32Ty(Context), 0));

  // Execute the module
  ExecutionEngine *EE = EngineBuilder(std::make_unique<Module>(M)).create();
  EE->finalizeObject();
  std::vector<GenericValue> Args;
  GenericValue Result = EE->runFunction(MainFunc, Args);

  return 0;
}