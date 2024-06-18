#include "Optimization/LoopFusion.h"
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

TEST(LoopFusionTest, BasicFusion) {
  LLVMContext Context;
  SMDiagnostic Error;
  std::unique_ptr<Module> M = parseIRFile("loop_fusion_input.ll", Error, Context);
  ASSERT_TRUE(M) << "Failed to parse IR file: " << Error.getMessage().str();

  legacy::PassManager PM;
  PM.add(createLoopFusionPass());
  PM.run(*M);

  std::string Expected = R"(
    define void @fusionTest() {
    entry:
      br label %loop_1

    loop_1:
      %i = phi i32 [ 0, %entry ], [ %inc_i, %loop_1_latch ]
      %j = phi i32 [ 0, %entry ], [ %inc_j, %loop_1_latch ]
      %cond_i = icmp slt i32 %i, 10
      %inc_i = add i32 %i, 1
      %cond_j = icmp slt i32 %j, 10
      %inc_j = add i32 %j, 1
      br i1 %cond_i, label %loop_1_body, label %loop_1_exit

    loop_1_body:
      ; Fused loop body
      ; ...
      br label %loop_1_latch

    loop_1_latch:
      br i1 %cond_j, label %loop_1, label %loop_1_exit

    loop_1_exit:
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