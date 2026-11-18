#ifndef OPERATOR_MAP_H
#define OPERATOR_MAP_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include "ast.h"

auto emit_int_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                     llvm::Value* left, llvm::Value* right, bool is_signed)
    -> llvm::Value*;

auto emit_float_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                       llvm::Value* left, llvm::Value* right) -> llvm::Value*;

auto emit_bool_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                      llvm::Value* left, llvm::Value* right) -> llvm::Value*;

auto emit_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                 llvm::Value* left, llvm::Value* right) -> llvm::Value*;

#endif  // OPERATOR_MAP_H
