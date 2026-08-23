#ifndef AST_H
#define AST_H

#include <boost/multiprecision/cpp_int.hpp>
namespace mp = boost::multiprecision;
#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

#include "source_location.h"
#include "token_type.h"

// forward declaration
struct Visitor;

enum class TypeId : uint8_t {
  Bool,
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
  String,
  IntLiteral,
  FloatLiteral,
  UserDefined,
};

const std::unordered_map<std::string, TypeId> builtin_types = {
    {"Bool", TypeId::Bool},
    {"Int8", TypeId::Int8},
    {"Int16", TypeId::Int16},
    {"Int32", TypeId::Int32},
    {"Int64", TypeId::Int64},
    {"UInt8", TypeId::UInt8},
    {"UInt16", TypeId::UInt16},
    {"UInt32", TypeId::UInt32},
    {"UInt64", TypeId::UInt64},
    {"Float32", TypeId::Float32},
    {"Float64", TypeId::Float64},
    {"String", TypeId::String},
    {"UserDefined", TypeId::UserDefined},
};

auto get_type_id(const std::string& s) -> TypeId;

const std::unordered_map<TypeId, std::string> type_id_str_map = {
    {TypeId::Bool, "Bool"},
    {TypeId::Int8, "Int8"},
    {TypeId::Int16, "Int16"},
    {TypeId::Int32, "Int32"},
    {TypeId::Int64, "Int64"},
    {TypeId::UInt8, "UInt8"},
    {TypeId::UInt16, "UInt16"},
    {TypeId::UInt32, "UInt32"},
    {TypeId::UInt64, "UInt64"},
    {TypeId::Float32, "Float32"},
    {TypeId::Float64, "Float64"},
    {TypeId::String, "String"},
    {TypeId::UserDefined, "UserDefined"},
};

auto type_id_to_str(TypeId type_id) -> std::string;

struct Type {
  TypeId type_id;
  std::string identifier;
};

constexpr auto BOOL_TYPE = Type{.type_id = TypeId::Bool, .identifier = "Bool"};
constexpr auto INT8_TYPE = Type{.type_id = TypeId::Int8, .identifier = "Int8"};
constexpr auto INT16_TYPE =
    Type{.type_id = TypeId::Int16, .identifier = "Int16"};
constexpr auto INT32_TYPE =
    Type{.type_id = TypeId::Int32, .identifier = "Int32"};
constexpr auto INT64_TYPE =
    Type{.type_id = TypeId::Int64, .identifier = "Int64"};
constexpr auto UINT8_TYPE =
    Type{.type_id = TypeId::UInt8, .identifier = "UInt8"};
constexpr auto UINT16_TYPE =
    Type{.type_id = TypeId::UInt16, .identifier = "UInt16"};
constexpr auto UINT32_TYPE =
    Type{.type_id = TypeId::UInt32, .identifier = "UInt32"};
constexpr auto UINT64_TYPE =
    Type{.type_id = TypeId::UInt64, .identifier = "UInt64"};
constexpr auto INT_LITERAL_TYPE =
    Type{.type_id = TypeId::IntLiteral, .identifier = "IntLiteral"};
constexpr auto FLOAT_LITERAL_TYPE =
    Type{.type_id = TypeId::FloatLiteral, .identifier = "FloatLiteral"};
constexpr auto FLOAT32_TYPE =
    Type{.type_id = TypeId::Float32, .identifier = "Float32"};
constexpr auto FLOAT64_TYPE =
    Type{.type_id = TypeId::Float64, .identifier = "Float64"};

enum class NodeKind : uint8_t {
  FloatLiteralExpression,
  BinaryExpression,
  VariableExpression,
  VariableAssignmentStatement,
  VariableDeclarationStatement,
  ShowStatement,
  FunctionDeclaration,
  FunctionCallExpression,
  Program,
  ReturnStatement,
  TypeExpression,
  IntLiteralExpression,
  IfStatement,
};

/*
   Expressions produce values, and so have resolved_type's
   Statements do NOT produce values
 */

class ASTNode {
 public:
  ASTNode(SourceLocation source_location, NodeKind kind)
      : source_location(source_location), kind_(kind) {}

  virtual ~ASTNode() = default;
  virtual auto accept(Visitor& v) -> void = 0;
  auto kind() -> NodeKind { return this->kind_; };
  SourceLocation source_location;

 private:
  NodeKind kind_;
};

struct FloatLiteralExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::FloatLiteralExpression;

  FloatLiteralExpression(double value, SourceLocation source_location)
      : ASTNode(source_location, KIND), value(value) {}

  double value;
  std::optional<Type> resolved_type;
  void accept(Visitor& v) override;
};

struct BinaryExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::BinaryExpression;

  BinaryExpression(TokenType op, std::unique_ptr<ASTNode> lhs,
                   std::unique_ptr<ASTNode> rhs, SourceLocation source_location)
      : ASTNode(source_location, KIND),
        op(op),
        lhs(std::move(lhs)),
        rhs(std::move(rhs)) {}

  TokenType op;
  std::unique_ptr<ASTNode> lhs, rhs;
  std::optional<Type> operand_type;
  std::optional<Type> resolved_type;
  void accept(Visitor& v) override;
};

struct VariableExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::VariableExpression;

  VariableExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  std::optional<Type> resolved_type;
  void accept(Visitor& v) override;
};

struct VariableAssignmentStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::VariableAssignmentStatement;

  VariableAssignmentStatement(std::string name, std::unique_ptr<ASTNode> value,
                              SourceLocation source_location)
      : ASTNode(source_location, NodeKind::VariableAssignmentStatement),
        name(std::move(name)),
        value(std::move(value)) {}

  std::string name;
  std::unique_ptr<ASTNode> value;
  std::optional<Type> assignment_type;
  void accept(Visitor& v) override;
};

struct VariableDeclarationStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::VariableDeclarationStatement;

  VariableDeclarationStatement(
      std::string name, std::unique_ptr<ASTNode> value,
      SourceLocation source_location,
      std::unique_ptr<ASTNode> type_identifier = nullptr)
      : ASTNode(source_location, KIND),
        name(std::move(name)),
        value(std::move(value)),
        type_identifier(std::move(type_identifier)) {}

  std::string name;
  std::unique_ptr<ASTNode> value;
  std::unique_ptr<ASTNode> type_identifier;
  std::optional<Type> declaration_type;
  void accept(Visitor& v) override;
};

struct ShowStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::ShowStatement;

  ShowStatement(std::unique_ptr<ASTNode> expr, SourceLocation source_location)
      : ASTNode(source_location, KIND), expr(std::move(expr)) {}

  std::unique_ptr<ASTNode> expr;
  std::optional<Type> expr_type;
  void accept(Visitor& v) override;
};

struct FunctionDeclaration : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::FunctionDeclaration;

  FunctionDeclaration(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  std::vector<std::unique_ptr<ASTNode>> statements;
  void accept(Visitor& v) override;
};

struct FunctionCallExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::FunctionCallExpression;

  FunctionCallExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  void accept(Visitor& v) override;
};

struct Program : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::Program;

  Program() : ASTNode(SourceLocation{}, KIND) {}
  std::vector<std::unique_ptr<ASTNode>> functions;
  void accept(Visitor& v) override;
};

struct ReturnStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::ReturnStatement;

  ReturnStatement(std::unique_ptr<ASTNode> expr, SourceLocation source_location)
      : ASTNode(source_location, KIND), expr(std::move(expr)) {}

  std::unique_ptr<ASTNode> expr;
  std::optional<Type> expr_type;
  void accept(Visitor& v) override;
};

struct TypeExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::TypeExpression;

  TypeExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  void accept(Visitor& v) override;
};

struct IntLiteralExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::IntLiteralExpression;

  IntLiteralExpression(mp::int128_t value, SourceLocation source_location)
      : ASTNode(source_location, KIND), value(std::move(value)) {}

  mp::int128_t value;
  std::optional<Type> resolved_type;
  void accept(Visitor& v) override;
};

struct IfStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::IfStatement;
  IfStatement(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  std::unique_ptr<ASTNode> condition;
  std::vector<std::unique_ptr<ASTNode>> body;
  std::vector<std::unique_ptr<ASTNode>> else_body;
  void accept(Visitor& v) override;
};

template <class T>
auto cast(ASTNode* node) -> T* {
  assert(node->kind() == T::KIND);

  return static_cast<T*>(node);
}

#endif  // AST_H
