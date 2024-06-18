#pragma once

#include "llvm/IR/Module.h"

namespace llvm {

class Function;
class Type;

/// Create a max pooling function.
/// \param M The module in which to create the function.
/// \param InputTy The type of the input tensor.
/// \param OutputTy The type of the output tensor.
/// \param KernelH The height of the pooling kernel.
/// \param KernelW The width of the pooling kernel.
/// \param StrideH The stride in the height dimension.
/// \param StrideW The stride in the width dimension.
/// \return The created max pooling function.
Function *createMaxPoolingFunction(Module &M, Type *InputTy, Type *OutputTy,
                                   unsigned KernelH, unsigned KernelW, unsigned StrideH, unsigned StrideW);

} // namespace llvm