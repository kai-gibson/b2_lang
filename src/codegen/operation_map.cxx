#include "codegen/operation_map.h"

#include "compile_error.h"

auto emit_int_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                     llvm::Value* left, llvm::Value* right, bool is_signed)
    -> llvm::Value* {
  switch (expr.op) {
    case BinaryOperation::Add:
      return builder.CreateAdd(left, right, "addtmp");
    case BinaryOperation::Subtract:
      return builder.CreateSub(left, right, "subtmp");
    case BinaryOperation::Multiply:
      return builder.CreateMul(left, right, "multmp");
    case BinaryOperation::Divide:
      return is_signed ? builder.CreateSDiv(left, right, "divtmp")
                       : builder.CreateUDiv(left, right, "udivtmp");
    case BinaryOperation::Equal:
      return builder.CreateICmpEQ(left, right, "eqtmp");
    case BinaryOperation::Greater:
      return is_signed ? builder.CreateICmpSGT(left, right, "gttmp")
                       : builder.CreateICmpUGT(left, right, "ugttmp");
    case BinaryOperation::GreaterEqual:
      return is_signed ? builder.CreateICmpSGE(left, right, "gtetmp")
                       : builder.CreateICmpUGE(left, right, "gtetmp");
    case BinaryOperation::Less:
      return is_signed ? builder.CreateICmpSLT(left, right, "lttmp")
                       : builder.CreateICmpULT(left, right, "lttmp");
    case BinaryOperation::LessEqual:
      return is_signed ? builder.CreateICmpSLE(left, right, "ltetmp")
                       : builder.CreateICmpULE(left, right, "ltetmp");
    case BinaryOperation::NotEqual:
      return builder.CreateICmpNE(left, right, "netmp");
    default:
      throw CodegenError(expr.source_location,
                         "Operation {} invalid for type {}", expr.op,
                         expr.resolved_type->kind());
  }
}

auto emit_float_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                       llvm::Value* left, llvm::Value* right) -> llvm::Value* {
  switch (expr.op) {
    case BinaryOperation::Add:
      return builder.CreateFAdd(left, right, "faddtmp");
    case BinaryOperation::Subtract:
      return builder.CreateFSub(left, right, "fsubtmp");
    case BinaryOperation::Multiply:
      return builder.CreateFMul(left, right, "fmultmp");
    case BinaryOperation::Divide:
      return builder.CreateFDiv(left, right, "fdivtmp");
    case BinaryOperation::Equal:
      return builder.CreateFCmpOEQ(left, right, "feqtmp");
    case BinaryOperation::Greater:
      return builder.CreateFCmpOGT(left, right, "fgttmp");
    case BinaryOperation::GreaterEqual:
      return builder.CreateFCmpOGE(left, right, "fgetmp");
    case BinaryOperation::Less:
      return builder.CreateFCmpOLT(left, right, "flttmp");
    case BinaryOperation::LessEqual:
      return builder.CreateFCmpOLE(left, right, "fletmp");
    case BinaryOperation::NotEqual:
      return builder.CreateFCmpUNE(left, right, "fnetmp");
    default:
      throw CodegenError(expr.source_location,
                         "Operation {} invalid for type {}", expr.op,
                         expr.resolved_type->kind());
  }
}

auto emit_bool_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                      llvm::Value* left, llvm::Value* right) -> llvm::Value* {
  switch (expr.op) {
    case BinaryOperation::Equal:
      return builder.CreateICmpEQ(left, right, "eqtmp");
    case BinaryOperation::NotEqual:
      return builder.CreateICmpNE(left, right, "netmp");
    case BinaryOperation::And:
      return builder.CreateAnd(left, right, "andtmp");
    case BinaryOperation::Or:
      return builder.CreateOr(left, right, "ortmp");
    default:
      throw CodegenError(expr.source_location,
                         "Operation {} invalid for type {}", expr.op,
                         expr.resolved_type->kind());
  }
}

auto emit_binary(llvm::IRBuilder<>& builder, BinaryExpression& expr,
                 llvm::Value* left, llvm::Value* right) -> llvm::Value* {
  auto type_flags = expr.operand_type->flags();

  if (type_flags.test(TypeFlag::Integer)) {
    return emit_int_binary(builder, expr, left, right,
                           type_flags.test(TypeFlag::Signed));
  } else if (type_flags.test(TypeFlag::Float)) {
    return emit_float_binary(builder, expr, left, right);
  } else if (type_flags.test(TypeFlag::Bool)) {
    return emit_bool_binary(builder, expr, left, right);
  } else {
    throw CodegenError(expr.source_location,
                       "Type {} does not support binary operation {}",
                       expr.resolved_type->kind(), expr.op);
  }
}

auto emit_int_unary(llvm::IRBuilder<>& builder, UnaryExpression& expr,
                    llvm::Value* operand) -> llvm::Value* {
  switch (expr.op) {
    case UnaryOperation::Negative:
    case UnaryOperation::AddressOf:
      return builder.CreateNeg(operand, "negtmp");
    case UnaryOperation::Not:
      throw CodegenError(expr.source_location,
                         "Type {} does not support binary operation {}",
                         expr.resolved_type->kind(), expr.op);
  }
}

auto emit_float_unary(llvm::IRBuilder<>& builder, UnaryExpression& expr,
                      llvm::Value* operand) -> llvm::Value* {
  switch (expr.op) {
    case UnaryOperation::Negative:
    case UnaryOperation::AddressOf:
      return builder.CreateFNeg(operand, "negtmp");
    case UnaryOperation::Not:
      throw CodegenError(expr.source_location,
                         "Type {} does not support binary operation {}",
                         expr.resolved_type->kind(), expr.op);
  }
}

auto emit_bool_unary(llvm::IRBuilder<>& builder, UnaryExpression& expr,
                     llvm::Value* operand) -> llvm::Value* {
  switch (expr.op) {
    case UnaryOperation::Negative:
    case UnaryOperation::AddressOf:
      throw CodegenError(expr.source_location,
                         "Type {} does not support binary operation {}",
                         expr.resolved_type->kind(), expr.op);
    case UnaryOperation::Not:
      return builder.CreateNot(operand, "nottmp");
  }
}

auto emit_unary(llvm::IRBuilder<>& builder, UnaryExpression& expr,
                llvm::Value* operand) -> llvm::Value* {
  auto type_flags = expr.resolved_type->flags();
  if (type_flags.test(TypeFlag::Integer)) {
    return emit_int_unary(builder, expr, operand);
  } else if (type_flags.test(TypeFlag::Float)) {
    return emit_float_unary(builder, expr, operand);
  } else if (type_flags.test(TypeFlag::Bool)) {
    return emit_bool_unary(builder, expr, operand);
  } else {
    throw CodegenError(expr.source_location,
                       "Type {} does not support binary operation {}",
                       expr.resolved_type->kind(), expr.op);
  }
}
