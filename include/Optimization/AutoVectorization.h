#pragma once

#include "llvm/IR/PassManager.h"

namespace llvm {

class FunctionPass;

/// Create an auto-vectorization pass.
/// This pass automatically vectorizes loops that can be safely and efficiently executed using SIMD instructions.
/// \return The created auto-vectorization pass.
FunctionPass *createAutoVectorizationPass();

} // namespace llvm
