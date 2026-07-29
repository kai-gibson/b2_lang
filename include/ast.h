#ifndef AST_H
#define AST_H

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
  IntLiteral,    // temporary type
  FloatLiteral,  // temporary type
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

class ASTNode {
 public:
  virtual ~ASTNode() = default;
  virtual void accept(Visitor& v) = 0;

  ASTNode(SourceLocation source_location) : source_location(source_location) {}
  std::optional<Type> resolved_type;
  std::optional<Type> binary_resolved_type;
  SourceLocation source_location;
};

struct FloatLiteralExpression : public ASTNode {
  FloatLiteralExpression(double value, SourceLocation source_location)
      : ASTNode(source_location), value(value) {}
  double value;

  void accept(Visitor& v) override;
};

struct BinaryExpression : public ASTNode {
  BinaryExpression(TokenType op, std::unique_ptr<ASTNode> lhs,
                   std::unique_ptr<ASTNode> rhs, SourceLocation source_location)
      : ASTNode(source_location),
        op(op),
        lhs(std::move(lhs)),
        rhs(std::move(rhs)) {}
  TokenType op;
  std::unique_ptr<ASTNode> lhs, rhs;

  void accept(Visitor& v) override;
};

struct VariableExpression : public ASTNode {
  VariableExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location), name(std::move(name)) {}

  std::string name;
  void accept(Visitor& v) override;
};

struct VariableAssignmentStatement : public ASTNode {
  VariableAssignmentStatement(std::string name, std::unique_ptr<ASTNode> value,
                              SourceLocation source_location)
      : ASTNode(source_location),
        name(std::move(name)),
        value(std::move(value)) {}

  std::string name;
  std::unique_ptr<ASTNode> value;
  void accept(Visitor& v) override;
};

struct VariableDeclarationStatement : public ASTNode {
  VariableDeclarationStatement(
      std::string name, std::unique_ptr<ASTNode> value,
      SourceLocation source_location,
      std::unique_ptr<ASTNode> type_identifier = nullptr)
      : ASTNode(source_location),
        name(std::move(name)),
        value(std::move(value)),
        type_identifier(std::move(type_identifier)) {}

  std::string name;
  std::unique_ptr<ASTNode> value;
  std::unique_ptr<ASTNode> type_identifier;
  void accept(Visitor& v) override;
};

struct ShowStatement : public ASTNode {
  ShowStatement(std::unique_ptr<ASTNode> expr, SourceLocation source_location)
      : ASTNode(source_location), expr(std::move(expr)) {}

  std::unique_ptr<ASTNode> expr;
  void accept(Visitor& v) override;
};

struct FunctionDeclaration : public ASTNode {
  FunctionDeclaration(std::string name, SourceLocation source_location)
      : ASTNode(source_location), name(std::move(name)) {}

  std::string name;
  std::vector<std::unique_ptr<ASTNode>> statements;

  void accept(Visitor& v) override;
};

struct FunctionCallExpression : public ASTNode {
  FunctionCallExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location), name(std::move(name)) {}

  std::string name;

  void accept(Visitor& v) override;
};

struct Program : public ASTNode {
  Program() : ASTNode(SourceLocation{}) {}
  std::vector<std::unique_ptr<ASTNode>> functions;

  void accept(Visitor& v) override;
};

struct ReturnStatement : public ASTNode {
  ReturnStatement(std::unique_ptr<ASTNode> value,
                  SourceLocation source_location)
      : ASTNode(source_location), value(std::move(value)) {}

  std::unique_ptr<ASTNode> value;

  void accept(Visitor& v) override;
};

struct TypeExpression : public ASTNode {
  TypeExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location), name(std::move(name)) {}

  std::string name;

  void accept(Visitor& v) override;
};

struct IntLiteralExpression : public ASTNode {
  IntLiteralExpression(int32_t value, SourceLocation source_location)
      : ASTNode(source_location), value(value) {}

  int32_t value;
  void accept(Visitor& v) override;
};

struct IfStatement : public ASTNode {
  IfStatement(SourceLocation source_location) : ASTNode(source_location) {}

  std::unique_ptr<ASTNode> condition;
  std::vector<std::unique_ptr<ASTNode>> body;

  void accept(Visitor& v) override;
};

#endif  // AST_H
