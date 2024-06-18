#include "Optimization/DataLayoutTransform.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

TEST(DataLayoutTransformTest, BasicTransform) {
  LLVMContext Context;
  SMDiagnostic Error;
  std::unique_ptr<Module> M = parseIRFile("data_layout_transform_input.ll", Error, Context);
  ASSERT_TRUE(M) << "Failed to parse IR file: " << Error.getMessage().str();

  legacy::PassManager PM;
  PM.add(createDataLayoutTransformPass());
  PM.run(*M);

  std::string Expected = R"(
    %struct.S = type { i32, i8, i16 }

    define void @transformTest(%struct.S* %s) {
    entry:
      %s1 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 0
      store i32 1, i32* %s1, align 4
      %s2 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 1
      store i8 2, i8* %s2, align 1
      %s3 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 2
      store i16 3, i16* %s3, align 2
      ret void
    }
  )";

  std::string Actual;
  raw_string_ostream OS(Actual);
  M->print(OS, nullptr);
  OS.flush();

  EXPECT_EQ(Expected, Actual);
}

} // namespace