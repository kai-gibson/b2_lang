#include "types.h"

auto type_factory(TypeKind t) -> std::unique_ptr<Type> {
  switch (t) {
    case TypeKind::Int8:
      return std::make_unique<Int8Type>();
    case TypeKind::Int16:
      return std::make_unique<Int16Type>();
    case TypeKind::Int32:
      return std::make_unique<Int32Type>();
    case TypeKind::Int64:
      return std::make_unique<Int64Type>();
    case TypeKind::UInt8:
      return std::make_unique<UInt8Type>();
    case TypeKind::UInt16:
      return std::make_unique<UInt16Type>();
    case TypeKind::UInt32:
      return std::make_unique<UInt32Type>();
    case TypeKind::UInt64:
      return std::make_unique<UInt64Type>();
    case TypeKind::Float32:
      return std::make_unique<Float32Type>();
    case TypeKind::Float64:
      return std::make_unique<Float64Type>();
    case TypeKind::IntLiteral:
      return std::make_unique<IntLiteralType>();
    case TypeKind::FloatLiteral:
      return std::make_unique<FloatLiteralType>();
    case TypeKind::String:
      return std::make_unique<StringType>();
    case TypeKind::Bool:
      return std::make_unique<BoolType>();
    case TypeKind::Pointer:
      return std::make_unique<PointerType>();
  }
}

auto str_to_type(const std::string& s) -> std::unique_ptr<Type> {
  auto t = ALL_TYPES.find(s);
  if (!t.has_value()) return nullptr;

  return type_factory(*t);
}

auto Type::flags() -> bitset<TypeFlag> {
  switch (this->kind()) {
    case TypeKind::Int8:
    case TypeKind::Int16:
    case TypeKind::Int32:
    case TypeKind::Int64:
      return {TypeFlag::Integer, TypeFlag::Signed};

    case TypeKind::UInt8:
    case TypeKind::UInt16:
    case TypeKind::UInt32:
    case TypeKind::UInt64:
    case TypeKind::IntLiteral:
      return {TypeFlag::Integer};

    case TypeKind::Float32:
    case TypeKind::Float64:
    case TypeKind::FloatLiteral:
      return {TypeFlag::Float};

    case TypeKind::Bool:
      return {TypeFlag::Bool};

    case TypeKind::Pointer:
      return {TypeFlag::Ptr};

    case TypeKind::String:
      return bitset<TypeFlag>{};
  }
}

auto binary_op_rule(BinaryOperation op) -> BinaryOpRule {
  switch (op) {
    case BinaryOperation::Add:
    case BinaryOperation::Subtract:
    case BinaryOperation::Multiply:
    case BinaryOperation::Divide:
      return {.return_type = BinaryReturnType::Same,
              .type_flags = {TypeFlag::Integer, TypeFlag::Float}};
    case BinaryOperation::Less:
    case BinaryOperation::LessEqual:
    case BinaryOperation::Greater:
    case BinaryOperation::GreaterEqual:
      return {.return_type = BinaryReturnType::Bool,
              .type_flags = {TypeFlag::Integer, TypeFlag::Float}};
    case BinaryOperation::Equal:
    case BinaryOperation::NotEqual:
      return {.return_type = BinaryReturnType::Bool,
              .type_flags = {TypeFlag::Bool, TypeFlag::Ptr, TypeFlag::Integer,
                             TypeFlag::Float}};
    case BinaryOperation::And:
    case BinaryOperation::Or:
      return {.return_type = BinaryReturnType::Bool,
              .type_flags = {TypeFlag::Bool}};
  }
}

auto set_all() -> bitset<TypeFlag> { return bitset<TypeFlag>().set(); }

auto unary_op_rule(UnaryOperation op) -> UnaryOpRule {
  switch (op) {
    case UnaryOperation::Negative:
      return {.return_type = UnaryReturnType::Same,
              .type_flags = {TypeFlag::Integer, TypeFlag::Float}};
    case UnaryOperation::Not:
      return {.return_type = UnaryReturnType::Same,
              .type_flags = {TypeFlag::Bool}};
  }
}
