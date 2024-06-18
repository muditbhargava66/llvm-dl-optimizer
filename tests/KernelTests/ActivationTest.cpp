#include "Kernels/Activation.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

TEST(ActivationTest, SimpleReLU) {
  LLVMContext Context;
  Module M("ActivationTestModule", Context);

  Type *FloatTy = Type::getFloatTy(Context);
  Type *InputTy = PointerType::get(FloatTy, 0);
  Type *OutputTy = PointerType::get(FloatTy, 0);

  Function *ReLUFunc = createReLUFunction(M, InputTy, OutputTy);
  ASSERT_TRUE(ReLUFunc != nullptr);

  // Create input tensor
  std::vector<float> InputData = {-1.0f, 2.0f, -3.0f, 4.0f, -5.0f, 6.0f};
  Constant *InputTensor = ConstantDataArray::get(Context, InputData);

  // Create output tensor
  std::vector<float> OutputData(6, 0.0f);
  GlobalVariable *OutputTensor = new GlobalVariable(M, ArrayType::get(FloatTy, 6), false,
                                                    GlobalValue::ExternalLinkage, nullptr, "output");
  OutputTensor->setInitializer(ConstantDataArray::get(Context, OutputData));

  // Create and run the test function
  Function *TestFunc = Function::Create(FunctionType::get(Type::getVoidTy(Context), false),
                                        GlobalValue::ExternalLinkage, "testReLU", &M);
  BasicBlock *BB = BasicBlock::Create(Context, "entry", TestFunc);
  IRBuilder<> Builder(BB);

  Value *InputPtr = Builder.CreatePointerCast(InputTensor, InputTy);
  Value *OutputPtr = Builder.CreatePointerCast(OutputTensor, OutputTy);
  Builder.CreateCall(ReLUFunc, {InputPtr, OutputPtr});
  Builder.CreateRetVoid();

  // Verify the module
  std::string ErrorMessage;
  raw_string_ostream ErrorStream(ErrorMessage);
  if (verifyModule(M, &ErrorStream)) {
    FAIL() << "Module verification failed: " << ErrorMessage;
  }

  // Compare the output tensor with the expected result
  std::vector<float> ExpectedOutput = {0.0f, 2.0f, 0.0f, 4.0f, 0.0f, 6.0f};
  Constant *ExpectedTensor = ConstantDataArray::get(Context, ExpectedOutput);
  EXPECT_TRUE(OutputTensor->getInitializer()->isElementWiseEqual(ExpectedTensor));
}

} // namespace