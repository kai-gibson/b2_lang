#include "codegen/operation_map.h"

#include <format>
#include "ast.h"

auto build_type_operations(Type type, llvm::IRBuilder<>& builder)
    -> std::unique_ptr<TypeOperations> {
  switch (type.type_id) {
    case TypeId::Int8:
    case TypeId::Int16:
    case TypeId::Int32:
    case TypeId::Int64:
      return std::make_unique<IntOperations>(builder, true);

    case TypeId::UInt8:
    case TypeId::UInt16:
    case TypeId::UInt32:
    case TypeId::UInt64:
      return std::make_unique<IntOperations>(builder, false);

    case TypeId::Float32:
    case TypeId::Float64:
      return std::make_unique<FloatOperations>(builder);

    case TypeId::IntLiteral:
    case TypeId::FloatLiteral:
      throw std::runtime_error(
          "Literal types should not exist by codegen time.");
    case TypeId::Sentinel:
          throw std::runtime_error(
          "Uninitialised type ID found.. This is bad");
    case TypeId::Bool:
    case TypeId::String:
    case TypeId::UserDefined:
      throw std::runtime_error(std::format(
          "Datatype {} operations not implemented", type.identifier));

  }
}

auto TypeOperations::apply(TokenType token_type, llvm::Value* lhs,
                           llvm::Value* rhs) -> llvm::Value* {
  switch (token_type) {
    case TokenType::Plus:
      return add(lhs, rhs);
    case TokenType::Minus:
      return subtract(lhs, rhs);
    case TokenType::Asterisk:
      return multiply(lhs, rhs);
    case TokenType::ForwardSlash:
      return divide(lhs, rhs);
    case TokenType::GreaterThan:
      return gt(lhs, rhs);
    case TokenType::LessThan:
      return lt(lhs, rhs);
    case TokenType::GreaterThanEquals:
      return gte(lhs, rhs);
    case TokenType::LessThanEquals:
      return lte(lhs, rhs);
    case TokenType::Equals:
      return eq(lhs, rhs);
    default:
      throw std::runtime_error(std::format("Unsupported operation: {}",
                                           token_type_to_str(token_type)));
  }
}

auto IntOperations::add(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateAdd(lhs, rhs, "addtmp");
}

auto IntOperations::subtract(llvm::Value* lhs, llvm::Value* rhs)
    -> llvm::Value* {
  return builder.CreateSub(lhs, rhs, "subtmp");
}

auto IntOperations::multiply(llvm::Value* lhs, llvm::Value* rhs)
    -> llvm::Value* {
  return builder.CreateMul(lhs, rhs, "multmp");
}

auto IntOperations::divide(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return is_signed ? builder.CreateSDiv(lhs, rhs, "divtmp")
                   : builder.CreateUDiv(lhs, rhs, "divtmp");
}

auto IntOperations::gt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return is_signed ? builder.CreateICmpSGT(lhs, rhs, "gttmp")
                   : builder.CreateICmpUGT(lhs, rhs, "gttmp");
}
auto IntOperations::lt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return is_signed ? builder.CreateICmpSLT(lhs, rhs, "lttmp")
                   : builder.CreateICmpULT(lhs, rhs, "lttmp");
}
auto IntOperations::gte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return is_signed ? builder.CreateICmpSGE(lhs, rhs, "gtetmp")
                   : builder.CreateICmpUGE(lhs, rhs, "gtetmp");
}

auto IntOperations::lte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return is_signed ? builder.CreateICmpSLE(lhs, rhs, "gtetmp")
                   : builder.CreateICmpULE(lhs, rhs, "gtetmp");
}

auto IntOperations::eq(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateICmpEQ(lhs, rhs, "eqtmp");
}

auto FloatOperations::add(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateFAdd(lhs, rhs, "faddtmp");
}

auto FloatOperations::subtract(llvm::Value* lhs, llvm::Value* rhs)
    -> llvm::Value* {
  return builder.CreateFSub(lhs, rhs, "fsubtmp");
}

auto FloatOperations::multiply(llvm::Value* lhs, llvm::Value* rhs)
    -> llvm::Value* {
  return builder.CreateFMul(lhs, rhs, "fmultmp");
}

auto FloatOperations::divide(llvm::Value* lhs, llvm::Value* rhs)
    -> llvm::Value* {
  return builder.CreateFDiv(lhs, rhs, "fdivtmp");
}

auto FloatOperations::gt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateFCmpOGT(lhs, rhs, "fgttmp");
}

auto FloatOperations::lt(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateFCmpOLT(lhs, rhs, "flttmp");
}

auto FloatOperations::gte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateFCmpOGE(lhs, rhs, "fgetmp");
}

auto FloatOperations::lte(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateFCmpOLE(lhs, rhs, "fletmp");
}

auto FloatOperations::eq(llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
  return builder.CreateFCmpOEQ(lhs, rhs, "fletmp");
}
