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
    case TypeKind::Product:
      return std::make_unique<ProductType>();
    case TypeKind::Array:
      return std::make_unique<ArrayType>();
    case TypeKind::Slice:
      return std::make_unique<SliceType>();
  }
}

auto str_to_type(const std::string& s) -> std::unique_ptr<Type> {
  auto t = ALL_TYPES.find(s);
  if (!t.has_value()) return nullptr;

  return type_factory(*t);
}
