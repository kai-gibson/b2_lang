#ifndef AST_H
#define AST_H

#include <boost/multiprecision/cpp_int.hpp>
namespace mp = boost::multiprecision;
#include <cassert>
#include <string>
#include <vector>

#include "source_location.h"
#include "token_type.h"
#include "types.h"

// forward declaration
struct Visitor;

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
  PointerTypeExpression,
  IntLiteralExpression,
  IfStatement,
  BlockStatement,
  LoopStatement,
  BreakStatement,
  CycleStatement,
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

struct BlockStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::BlockStatement;
  BlockStatement(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  std::vector<std::unique_ptr<ASTNode>> statements;
  void accept(Visitor& v) override;
};

struct FloatLiteralExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::FloatLiteralExpression;

  FloatLiteralExpression(double value, SourceLocation source_location)
      : ASTNode(source_location, KIND), value(value) {}

  double value;
  std::unique_ptr<Type> resolved_type;
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
  std::unique_ptr<Type> operand_type;
  std::unique_ptr<Type> resolved_type;
  void accept(Visitor& v) override;
};

struct VariableExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::VariableExpression;

  VariableExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  std::unique_ptr<Type> resolved_type;
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
  std::unique_ptr<Type> assignment_type;
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
  std::unique_ptr<Type> declaration_type;
  void accept(Visitor& v) override;
};

struct ShowStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::ShowStatement;

  ShowStatement(std::unique_ptr<ASTNode> expr, SourceLocation source_location)
      : ASTNode(source_location, KIND), expr(std::move(expr)) {}

  std::unique_ptr<ASTNode> expr;
  std::unique_ptr<Type> expr_type;
  void accept(Visitor& v) override;
};

struct FunctionDeclaration : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::FunctionDeclaration;

  FunctionDeclaration(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  std::unique_ptr<ASTNode> statements;
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
  std::unique_ptr<Type> expr_type;
  void accept(Visitor& v) override;
};

struct TypeExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::TypeExpression;

  TypeExpression(std::string name, SourceLocation source_location)
      : ASTNode(source_location, KIND), name(std::move(name)) {}

  std::string name;
  void accept(Visitor& v) override;
};

struct PointerTypeExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::PointerTypeExpression;

  PointerTypeExpression(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  std::unique_ptr<ASTNode> base_type;
  void accept(Visitor& v) override;
};

struct IntLiteralExpression : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::IntLiteralExpression;

  IntLiteralExpression(mp::int128_t value, SourceLocation source_location)
      : ASTNode(source_location, KIND), value(std::move(value)) {}

  mp::int128_t value;
  std::unique_ptr<Type> resolved_type;
  void accept(Visitor& v) override;
};

struct IfStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::IfStatement;
  IfStatement(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  std::unique_ptr<ASTNode> condition;
  std::unique_ptr<ASTNode> if_then;
  std::unique_ptr<ASTNode> if_else;
  void accept(Visitor& v) override;
};

struct LoopStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::LoopStatement;
  LoopStatement(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  std::unique_ptr<ASTNode> body;
  void accept(Visitor& v) override;
};

struct BreakStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::BreakStatement;
  BreakStatement(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  void accept(Visitor& v) override;
};

struct CycleStatement : public ASTNode {
  static constexpr NodeKind KIND = NodeKind::CycleStatement;
  CycleStatement(SourceLocation source_location)
      : ASTNode(source_location, KIND) {}

  void accept(Visitor& v) override;
};

template <class T>
auto cast(ASTNode* node) -> T* {
  assert(node->kind() == T::KIND);

  return static_cast<T*>(node);
}

#endif  // AST_H
