#pragma once

#include "llvm/IR/PassManager.h"

namespace llvm {

class FunctionPass;

/// Create a data layout transformation pass.
/// This pass optimizes the data layout of structures and arrays to improve memory access efficiency.
FunctionPass *createDataLayoutTransformPass();

} // namespace llvm