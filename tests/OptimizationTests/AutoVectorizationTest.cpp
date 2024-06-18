#include "Optimization/AutoVectorization.h"
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

TEST(AutoVectorizationTest, BasicVectorization) {
  LLVMContext Context;
  SMDiagnostic Error;
  std::unique_ptr<Module> M = parseIRFile("auto_vectorization_input.ll", Error, Context);
  ASSERT_TRUE(M) << "Failed to parse IR file: " << Error.getMessage().str();

  legacy::PassManager PM;
  PM.add(createAutoVectorizationPass());
  PM.run(*M);

  std::string Expected = R"(
    define void @vectorizationTest(i32* %a, i32* %b, i32* %c) {
    entry:
      br label %loop.header

    loop.header:
      %idx = phi i64 [ 0, %entry ], [ %idx.next, %loop.latch ]
      %idx.next = add i64 %idx, 4
      %cond = icmp slt i64 %idx.next, 1024
      br i1 %cond, label %loop.body, label %loop.exit

    loop.body:
      %a.ptr = getelementptr inbounds i32, i32* %a, i64 %idx
      %a.load = load <4 x i32>, <4 x i32>* %a.ptr, align 4
      %b.ptr = getelementptr inbounds i32, i32* %b, i64 %idx
      %b.load = load <4 x i32>, <4 x i32>* %b.ptr, align 4
      %add = add <4 x i32> %a.load, %b.load
      %c.ptr = getelementptr inbounds i32, i32* %c, i64 %idx
      store <4 x i32> %add, <4 x i32>* %c.ptr, align 4
      br label %loop.latch

    loop.latch:
      br i1 %cond, label %loop.header, label %loop.exit

    loop.exit:
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