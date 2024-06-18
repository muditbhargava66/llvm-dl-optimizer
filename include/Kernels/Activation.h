#pragma once

#include "llvm/IR/Module.h"

namespace llvm {

class Function;
class Type;

/// Create a ReLU activation function.
/// \param M The module in which to create the function.
/// \param InputTy The type of the input tensor.
/// \param OutputTy The type of the output tensor.
/// \return The created ReLU activation function.
Function *createReLUFunction(Module &M, Type *InputTy, Type *OutputTy);

} // namespace llvm