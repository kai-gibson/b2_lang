#ifndef OPERATOR_MAP_H
#define OPERATOR_MAP_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include "ast.h"

class TypeOperations {
 public:
  virtual ~TypeOperations() = default;
  virtual auto add(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto subtract(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto multiply(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto divide(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto gt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto lt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto gte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto lte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;
  virtual auto eq(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* = 0;

  auto apply(TokenType token_type, llvm::Value* lhs, llvm::Value* rhs)
      -> llvm::Value*;
};

class IntOperations : public TypeOperations {
 public:
  IntOperations(llvm::IRBuilder<>& builder, bool is_signed = true)
      : builder(builder), is_signed(is_signed) {}
  auto add(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto subtract(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto multiply(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto divide(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto gt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto lt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto gte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto lte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto eq(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;

 private:
  llvm::IRBuilder<>& builder;
  bool is_signed;
};

class FloatOperations : public TypeOperations {
 public:
  FloatOperations(llvm::IRBuilder<>& builder) : builder(builder) {}
  auto add(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto subtract(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto multiply(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto divide(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto gt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto lt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto gte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto lte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;
  auto eq(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* override;

 private:
  llvm::IRBuilder<>& builder;
};

class BoolOperations : public TypeOperations {
 public:
 private:
};

auto build_type_operations(Type type, llvm::IRBuilder<>& builder)
    -> std::unique_ptr<TypeOperations>;

#endif  // OPERATOR_MAP_H
