#include "Kernels/Convolution.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

TEST(ConvolutionTest, SimpleConvolution) {
  LLVMContext Context;
  Module M("ConvolutionTestModule", Context);

  Type *FloatTy = Type::getFloatTy(Context);
  Type *InputTy = PointerType::get(FloatTy, 0);
  Type *WeightTy = PointerType::get(FloatTy, 0);
  Type *OutputTy = PointerType::get(FloatTy, 0);

  Function *ConvFunc = createConvolutionFunction(M, InputTy, WeightTy, OutputTy, 1, 1, 0, 0);
  ASSERT_TRUE(ConvFunc != nullptr);

  // Create input and weight tensors
  std::vector<float> InputData = {1.0f, 2.0f, 3.0f, 4.0f};
  std::vector<float> WeightData = {0.5f, 0.5f, 0.5f, 0.5f};
  Constant *InputTensor = ConstantDataArray::get(Context, InputData);
  Constant *WeightTensor = ConstantDataArray::get(Context, WeightData);

  // Create output tensor
  std::vector<float> OutputData(4, 0.0f);
  GlobalVariable *OutputTensor = new GlobalVariable(M, ArrayType::get(FloatTy, 4), false,
                                                    GlobalValue::ExternalLinkage, nullptr, "output");
  OutputTensor->setInitializer(ConstantDataArray::get(Context, OutputData));

  // Create and run the test function
  Function *TestFunc = Function::Create(FunctionType::get(Type::getVoidTy(Context), false),
                                        GlobalValue::ExternalLinkage, "testConvolution", &M);
  BasicBlock *BB = BasicBlock::Create(Context, "entry", TestFunc);
  IRBuilder<> Builder(BB);

  Value *InputPtr = Builder.CreatePointerCast(InputTensor, InputTy);
  Value *WeightPtr = Builder.CreatePointerCast(WeightTensor, WeightTy);
  Value *OutputPtr = Builder.CreatePointerCast(OutputTensor, OutputTy);
  Builder.CreateCall(ConvFunc, {InputPtr, WeightPtr, OutputPtr});
  Builder.CreateRetVoid();

  // Verify the module
  std::string ErrorMessage;
  raw_string_ostream ErrorStream(ErrorMessage);
  if (verifyModule(M, &ErrorStream)) {
    FAIL() << "Module verification failed: " << ErrorMessage;
  }

  // Compare the output tensor with the expected result
  std::vector<float> ExpectedOutput = {2.5f, 3.5f, 4.5f, 5.5f};
  Constant *ExpectedTensor = ConstantDataArray::get(Context, ExpectedOutput);
  EXPECT_TRUE(OutputTensor->getInitializer()->isElementWiseEqual(ExpectedTensor));
}

} // namespace