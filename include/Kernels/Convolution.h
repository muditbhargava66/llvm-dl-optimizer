#pragma once

#include "llvm/IR/Module.h"

namespace llvm {

class Function;
class Type;

/// Create a convolution function.
/// \param M The module in which to create the function.
/// \param InputTy The type of the input tensor.
/// \param WeightTy The type of the weight tensor.
/// \param OutputTy The type of the output tensor.
/// \param StrideH The stride in the height dimension.
/// \param StrideW The stride in the width dimension.
/// \param PadH The padding in the height dimension.
/// \param PadW The padding in the width dimension.
/// \return The created convolution function.
Function *createConvolutionFunction(Module &M, Type *InputTy, Type *WeightTy, Type *OutputTy,
                                    unsigned StrideH, unsigned StrideW, unsigned PadH, unsigned PadW);

} // namespace llvm
