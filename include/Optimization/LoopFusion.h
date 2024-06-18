#pragma once

#include "llvm/IR/PassManager.h"

namespace llvm {

class FunctionPass;

/// Create a loop fusion pass.
/// This pass fuses adjacent loops that have no dependencies and can be safely merged.
/// \return The created loop fusion pass.
FunctionPass *createLoopFusionPass();

} // namespace llvm
