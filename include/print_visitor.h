#ifndef PRINT_VISITOR_H
#define PRINT_VISITOR_H
#include <format>
#include <string>

#include "ast.h"
#include "visitor.h"

/*
  stack based pretty printer for the AST in s-expression format

  Format is
  (Node value
    (Child value))
 */
struct PrettyPrinter {
  struct Closer {
    PrettyPrinter& printer;

    ~Closer() { printer.close_node(); }
  };

  auto add_node(std::string_view name) -> Closer {
    if (depth) {
      output += std::format("\n{}", std::string(depth * 4, ' '));
    }

    output += std::format("({}", name);
    depth += 1;

    return Closer{*this};
  }

  void add_value(std::string_view value) {
    output += std::format(" {}", value);
  }

  void add_string_value(std::string_view value) {
    output += std::format(" \"{}\"", value);
  }

  void close_node() {
    depth -= 1;
    output += ")";
  }

  auto to_string() -> std::string { return std::move(output); }

  uint64_t depth{};
  std::string output;
};

struct PrintVisitor : public Visitor {
  PrettyPrinter printer;

  void print_type(const std::optional<Type>& resolved_type,
                  const std::string& name = "ResolvedType") {
    if (resolved_type) {
      auto print_node = printer.add_node(name);
      printer.add_value(resolved_type->identifier);
    }
  }

  void visit(FloatLiteralExpression& expr) override {
    auto print_node = printer.add_node("FloatLiteral");
    printer.add_value(std::to_string(expr.value));
    print_type(expr.resolved_type);
  }

  void visit(IntLiteralExpression& expr) override {
    auto print_node = printer.add_node("IntLiteralExpression");
    printer.add_value(expr.value.str());
    print_type(expr.resolved_type);
  }

  void visit(BinaryExpression& expr) override {
    auto print_node = printer.add_node("BinaryExpression");
    printer.add_value(token_type_to_str(expr.op));
    print_type(expr.resolved_type);
    print_type(expr.operand_type, "OperandType");

    expr.lhs->accept(*this);
    expr.rhs->accept(*this);
  }

  void visit(VariableExpression& expr) override {
    auto print_node = printer.add_node("VariableExpression");
    printer.add_string_value(expr.name);
    print_type(expr.resolved_type);
  }

  void visit(VariableDeclarationStatement& stmt) override {
    auto print_node = printer.add_node("VariableDeclarationStatement");
    printer.add_string_value(stmt.name);
    print_type(stmt.declaration_type, "DeclarationType");

    stmt.value->accept(*this);

    if (stmt.type_identifier) {
      stmt.type_identifier->accept(*this);
    }
  }

  void visit(VariableAssignmentStatement& stmt) override {
    auto print_node = printer.add_node("VarAssign");
    printer.add_string_value(stmt.name);
    stmt.value->accept(*this);
  }

  void visit(ShowStatement& stmt) override {
    auto print_node = printer.add_node("Show");
    stmt.expr->accept(*this);
  }

  void visit(FunctionDeclaration& func) override {
    auto print_node = printer.add_node("FunctionDeclaration");
    printer.add_string_value(func.name);

    func.statements->accept(*this);
  }

  void visit(FunctionCallExpression& call) override {
    auto print_node = printer.add_node("FunctionCall");
    printer.add_string_value(call.name);
  }

  void visit(Program& program) override {
    auto print_node = printer.add_node("Program");
    for (const auto& func : program.functions) {
      func->accept(*this);
    }
  }

  void visit(ReturnStatement& ret) override {
    auto print_node = printer.add_node("ReturnStatement");
    ret.expr->accept(*this);
  }

  void visit(TypeExpression& ret) override {
    auto print_node = printer.add_node("TypeExpression");
    printer.add_value(ret.name);
  }

  void visit(IfStatement& ifstmt) override {
    auto print_node = printer.add_node("IfStatement");
    {
      auto cond = printer.add_node("Cond");
      ifstmt.condition->accept(*this);
    }

    {
      auto if_then = printer.add_node("IfThen");
      ifstmt.if_then->accept(*this);
    }

    if (ifstmt.if_else) {
      auto else_then = printer.add_node("IfElse");
      ifstmt.if_else->accept(*this);
    }
  }

  void visit(BlockStatement& block) override {
    for (const auto& stmt : block.statements) {
      stmt->accept(*this);
    }
  }
};

#endif  // PRINT_VISITOR_H
