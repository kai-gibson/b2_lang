#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <string>
#include <vector>

#include "utils/container.h"

enum class TypeKind : uint8_t {
  Int8,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  Float32,
  Float64,
  IntLiteral,
  FloatLiteral,
  String,
  Bool,
  Pointer,
  Product,
  Array,
  Slice,
};

constexpr auto TYPE_KIND_STR = to_static_map<TypeKind, std::string_view>({
    {TypeKind::Int8, "Int8"},
    {TypeKind::Int16, "Int16"},
    {TypeKind::Int32, "Int32"},
    {TypeKind::Int64, "Int64"},
    {TypeKind::UInt8, "UInt8"},
    {TypeKind::UInt16, "UInt16"},
    {TypeKind::UInt32, "UInt32"},
    {TypeKind::UInt64, "UInt64"},
    {TypeKind::Float32, "Float32"},
    {TypeKind::Float64, "Float64"},
    {TypeKind::IntLiteral, "IntLiteral"},
    {TypeKind::FloatLiteral, "FloatLiteral"},
    {TypeKind::String, "String"},
    {TypeKind::Bool, "Bool"},
    {TypeKind::Pointer, "Pointer"},
    {TypeKind::Product, "Product"},
    {TypeKind::Array, "Array"},
    {TypeKind::Slice, "Slice"},
});

constexpr auto PRIMITIVE_TYPES = to_static_map<std::string_view, TypeKind>({
    {"Bool", TypeKind::Bool},
    {"Int8", TypeKind::Int8},
    {"Int16", TypeKind::Int16},
    {"Int32", TypeKind::Int32},
    {"Int64", TypeKind::Int64},
    {"UInt8", TypeKind::UInt8},
    {"UInt16", TypeKind::UInt16},
    {"UInt32", TypeKind::UInt32},
    {"UInt64", TypeKind::UInt64},
    {"Float32", TypeKind::Float32},
    {"Float64", TypeKind::Float64},
    {"String", TypeKind::String},
});

constexpr auto ALL_TYPES = to_static_map<std::string_view, TypeKind>({
    {"Int8", TypeKind::Int8},
    {"Int16", TypeKind::Int16},
    {"Int32", TypeKind::Int32},
    {"Int64", TypeKind::Int64},
    {"UInt8", TypeKind::UInt8},
    {"UInt16", TypeKind::UInt16},
    {"UInt32", TypeKind::UInt32},
    {"UInt64", TypeKind::UInt64},
    {"Float32", TypeKind::Float32},
    {"Float64", TypeKind::Float64},
    {"IntLiteral", TypeKind::IntLiteral},
    {"FloatLiteral", TypeKind::FloatLiteral},
    {"String", TypeKind::String},
    {"Bool", TypeKind::Bool},
    {"Pointer", TypeKind::Pointer},
    {"Product", TypeKind::Product},
    {"Array", TypeKind::Array},
    {"Slice", TypeKind::Slice},
});

constexpr auto type_kind_to_str(TypeKind t) -> const std::string_view {
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  return TYPE_KIND_STR.find(t)->get();
}

class Type {
 public:
  Type(TypeKind kind) : kind_(kind) {}
  virtual ~Type() = default;

  auto kind() -> TypeKind { return this->kind_; };
  auto kind_name() -> std::string_view { return type_kind_to_str(kind_); };

  virtual auto clone() -> std::unique_ptr<Type> = 0;

 private:
  TypeKind kind_;
};

struct PointerType : public Type {
  static constexpr TypeKind KIND = TypeKind::Pointer;
  PointerType() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    auto ptr = std::make_unique<PointerType>();
    ptr->base = this->base->clone();

    return ptr;
  }

  std::unique_ptr<Type> base;
};

struct Int8Type : public Type {
  static constexpr TypeKind KIND = TypeKind::Int8;
  Int8Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<Int8Type>();
  }
};

struct Int16Type : public Type {
  static constexpr TypeKind KIND = TypeKind::Int16;
  Int16Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<Int16Type>();
  }
};

struct Int32Type : public Type {
  static constexpr TypeKind KIND = TypeKind::Int32;
  Int32Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<Int32Type>();
  }
};

struct Int64Type : public Type {
  static constexpr TypeKind KIND = TypeKind::Int64;
  Int64Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<Int64Type>();
  }
};

struct UInt8Type : public Type {
  static constexpr TypeKind KIND = TypeKind::UInt8;
  UInt8Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<UInt8Type>();
  }
};

struct UInt16Type : public Type {
  static constexpr TypeKind KIND = TypeKind::UInt16;
  UInt16Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<UInt16Type>();
  }
};

struct UInt32Type : public Type {
  static constexpr TypeKind KIND = TypeKind::UInt32;
  UInt32Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<UInt32Type>();
  }
};

struct UInt64Type : public Type {
  static constexpr TypeKind KIND = TypeKind::UInt64;
  UInt64Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<UInt64Type>();
  }
};

struct Float32Type : public Type {
  static constexpr TypeKind KIND = TypeKind::Float32;
  Float32Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<Float32Type>();
  }
};

struct Float64Type : public Type {
  static constexpr TypeKind KIND = TypeKind::Float64;
  Float64Type() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<Float64Type>();
  }
};

struct IntLiteralType : public Type {
  static constexpr TypeKind KIND = TypeKind::IntLiteral;
  IntLiteralType() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<IntLiteralType>();
  }
};

struct FloatLiteralType : public Type {
  static constexpr TypeKind KIND = TypeKind::FloatLiteral;
  FloatLiteralType() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<FloatLiteralType>();
  }
};

struct StringType : public Type {
  static constexpr TypeKind KIND = TypeKind::String;
  StringType() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<StringType>();
  }
};

struct BoolType : public Type {
  static constexpr TypeKind KIND = TypeKind::Bool;
  BoolType() : Type(KIND) {}

  auto clone() -> std::unique_ptr<Type> override {
    return std::make_unique<BoolType>();
  }
};

struct ProductType : public Type {
  static constexpr TypeKind KIND = TypeKind::Product;
  ProductType() : Type(KIND) {}

  struct Field {
    std::string name;
    std::unique_ptr<Type> type;
  };

  std::vector<Field> fields;

  auto clone() -> std::unique_ptr<Type> override {
    auto product_type = std::make_unique<ProductType>();
    for (const auto& field : this->fields) {
      product_type->fields.push_back(
          Field{.name = field.name, .type = field.type->clone()});
    }

    return product_type;
  }
};

struct ArrayType : public Type {
  static constexpr TypeKind KIND = TypeKind::Array;
  ArrayType() : Type(KIND) {}

  size_t size;
  std::unique_ptr<Type> base;

  auto clone() -> std::unique_ptr<Type> override {
    auto arr = std::make_unique<ArrayType>();
    arr->base = this->base->clone();

    return arr;
  }
};

struct SliceType : public Type {
  static constexpr TypeKind KIND = TypeKind::Slice;
  SliceType() : Type(KIND) {}

  std::unique_ptr<Type> base;

  auto clone() -> std::unique_ptr<Type> override {
    auto arr = std::make_unique<SliceType>();
    arr->base = this->base->clone();

    return arr;
  }
};

auto type_factory(TypeKind t) -> std::unique_ptr<Type>;
auto str_to_type(const std::string& s) -> std::unique_ptr<Type>;

#endif  // TYPES_H
