#include "type_check_visitor.h"

#include <algorithm>
#include <iostream>

#include "compile_error.h"
#include "lexer.h"
#include "print_visitor.h"

auto is_int_type(TypeId type_id) -> bool {
  using std::ranges::contains;

  constexpr std::array int_types = {
      TypeId::Int8,  TypeId::Int16,  TypeId::Int32,  TypeId::Int64,
      TypeId::UInt8, TypeId::UInt16, TypeId::UInt32, TypeId::UInt64};

  return contains(int_types, type_id);
}

auto is_float_type(TypeId type_id) -> bool {
  using std::ranges::contains;

  constexpr std::array float_types = {TypeId::Float32, TypeId::Float64};

  return contains(float_types, type_id);
}

auto resolve_literal_type(std::optional<Type> explicit_type, Type& literal,
                          SourceLocation& source_location) -> Type {
  if (literal.type_id == TypeId::IntLiteral) {
    if (!explicit_type)
      return Type{.type_id = TypeId::Int32, .identifier = "Int32"};

    if (!is_int_type(explicit_type->type_id)) {
      throw TypeError(source_location, "Can't coerce int literal to type: {}",
                      explicit_type->identifier);
    }

    return *explicit_type;
  }

  if (literal.type_id == TypeId::FloatLiteral) {
    if (!explicit_type)
      return Type{.type_id = TypeId::Float64, .identifier = "Float64"};

    if (!is_float_type(explicit_type->type_id)) {
      throw TypeError(source_location, "Can't coerce float literal to type: {}",
                      explicit_type->identifier);
    }

    return *explicit_type;
  }

  throw std::runtime_error("Shouldn't have reached here.");
}

void TypeCheckVisitor::visit(BinaryExpression& expr) {
  auto lhs = emit(*expr.lhs);
  auto rhs = emit(*expr.rhs);

  bool lhs_literal =
      lhs.type_id == TypeId::IntLiteral || lhs.type_id == TypeId::FloatLiteral;
  bool rhs_literal =
      rhs.type_id == TypeId::IntLiteral || rhs.type_id == TypeId::FloatLiteral;

  // one is a literal, one is concrete — resolve the literal against the
  // concrete
  if (lhs_literal != rhs_literal) {
    if (lhs_literal) {
      lhs = resolve_literal_type(rhs, lhs, expr.source_location);
      expr.lhs->resolved_type = lhs;
    } else {
      rhs = resolve_literal_type(lhs, rhs, expr.source_location);
      expr.rhs->resolved_type = rhs;
    }
  }

  // default to Float64 or Int32
  if (lhs_literal && rhs_literal) {
    if (lhs.type_id == TypeId::IntLiteral &&
        rhs.type_id == TypeId::IntLiteral) {
      expr.lhs->resolved_type =
          Type{.type_id = TypeId::Int32, .identifier = "Int32"};
      expr.rhs->resolved_type = expr.lhs->resolved_type;

    } else if (lhs.type_id == TypeId::FloatLiteral ||
               rhs.type_id == TypeId::FloatLiteral) {
      expr.lhs->resolved_type =
          Type{.type_id = TypeId::Float64, .identifier = "Float64"};
      expr.rhs->resolved_type = expr.lhs->resolved_type;
    } else {
      throw TypeError(expr.source_location, "Unexpected non-numeric literal");
    }
  } else {
    std::cout << (int)lhs.type_id << " " << (int)rhs.type_id << "\n";
    if (lhs.type_id != rhs.type_id) {
      throw TypeError(
          expr.source_location,
          "Type Error: left ({}) and right ({}) of binary operation: {} do not "
          "match",
          lhs.identifier, rhs.identifier, token_type_to_str(expr.op));
    }
  }

  if (is_bool_operator(expr.op)) {
    result = Type{.type_id = TypeId::Bool, .identifier = "Bool"};
    expr.resolved_type = result;
    expr.binary_resolved_type = expr.lhs->resolved_type;
    // Type{.type_id = rhs.type_id, .identifier = rhs.identifier};

    return;
  }

  // infer from the right value
  result = Type{.type_id = rhs.type_id, .identifier = rhs.identifier};
  expr.resolved_type = result;
}

void TypeCheckVisitor::visit(FloatLiteralExpression& expr) {
  result = Type{.type_id = TypeId::FloatLiteral, .identifier = ""};
  expr.resolved_type = result;
}

void TypeCheckVisitor::visit(IntLiteralExpression& expr) {
  result = Type{.type_id = TypeId::IntLiteral, .identifier = ""};
  expr.resolved_type = result;
}

void TypeCheckVisitor::visit(VariableExpression& expr) {
  auto it = variable_map.find(expr.name);
  if (it == variable_map.end()) {
    throw TypeError(expr.source_location, "Error: variable: {} not declared",
                    expr.name);
  }

  result = it->second;
  expr.resolved_type = result;
}

void TypeCheckVisitor::visit(Program& program) {
  for (const auto& func : program.functions) {
    func->accept(*this);
  }
}

void TypeCheckVisitor::visit(VariableDeclarationStatement& stmt) {
  std::optional<Type> expected_type;

  if (stmt.type_identifier) {
    expected_type = emit(*stmt.type_identifier);
  }

  auto value = emit(*stmt.value);

  if (value.type_id == TypeId::IntLiteral ||
      value.type_id == TypeId::FloatLiteral) {
    value = resolve_literal_type(expected_type, value, stmt.source_location);
    stmt.value->resolved_type = value;
  }

  if (expected_type && expected_type->type_id != value.type_id) {
    throw TypeError(stmt.source_location,
                    "Type Error: variable: {} with type: {} does not match "
                    "expression type: {}",
                    stmt.name, expected_type->identifier, value.identifier);
  }

  // else just infer type from expression
  result = value;
  stmt.resolved_type = result;

  // save to symbol map
  variable_map[stmt.name] = result;
}

void TypeCheckVisitor::visit(VariableAssignmentStatement& stmt) {
  auto it = variable_map.find(stmt.name);
  if (it == variable_map.end()) {
    throw TypeError(stmt.source_location, "Error: variable: {} not declared",
                    stmt.name);
  }

  auto var_type = it->second;
  auto expr_type = emit(*stmt.value);

  if (expr_type.type_id == TypeId::IntLiteral ||
      expr_type.type_id == TypeId::FloatLiteral) {
    expr_type = resolve_literal_type(var_type, expr_type, stmt.source_location);
    stmt.value->resolved_type = expr_type;
  }

  if (var_type.type_id != expr_type.type_id) {
    throw TypeError(stmt.source_location,
                    "Type Error: cannot assign type \"{}\" to variable \"{}\" "
                    "of type \"{}\"",
                    expr_type.identifier, stmt.name, var_type.identifier);
  }
}

// no-op
void TypeCheckVisitor::visit(ShowStatement& stmt) { stmt.expr->accept(*this); }

void TypeCheckVisitor::visit(FunctionDeclaration& func) {
  // set current function
  current_function = &func;

  // resolve function return type to Int32 for now
  func.resolved_type = Type{.type_id = TypeId::Int32, .identifier = "Int32"};

  // scope variable type map to function
  variable_map.clear();
  for (const auto& stmt : func.statements) {
    stmt->accept(*this);
    if (result.type_id == TypeId::IntLiteral ||
        result.type_id == TypeId::FloatLiteral) {
      throw TypeError(stmt->source_location,
                      "Could not infer type of literal, add a type annotation");
    }
  }
}

void TypeCheckVisitor::visit(FunctionCallExpression& stmt) { (void)stmt; }

void TypeCheckVisitor::visit(ReturnStatement& stmt) {
  auto value = emit(*stmt.value);

  if (value.type_id == TypeId::IntLiteral ||
      value.type_id == TypeId::FloatLiteral) {
    value = resolve_literal_type(this->current_function->resolved_type, value,
                                 stmt.source_location);
    stmt.value->resolved_type = value;
  }

  result = value;
  stmt.resolved_type = result;
}

void TypeCheckVisitor::visit(TypeExpression& expr) {
  auto type_id = get_type_id(expr.name);
  result = Type{.type_id = type_id, .identifier = expr.name};
}

void TypeCheckVisitor::visit(IfStatement& if_stmt) {
  PrintVisitor print;
  auto cond = emit(*if_stmt.condition);

  if (cond.type_id != TypeId::Bool) {
    throw TypeError(if_stmt.condition->source_location,
                    "If statement condition must be of type Bool");
  }

  for (const auto& stmt : if_stmt.body) {
    stmt->accept(*this);
  }
}
