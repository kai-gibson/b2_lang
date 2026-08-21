#include "type_check_visitor.h"

#include <utility>

#include "compile_error.h"
#include "lexer.h"
#include "parser.h"

auto is_literal(Type& type) -> bool {
  switch (type.type_id) {
    case TypeId::IntLiteral:
    case TypeId::FloatLiteral:
      return true;
    default:
      return false;
  }
}

auto is_integer(Type& type) -> bool {
  switch (type.type_id) {
    case TypeId::Int8:
    case TypeId::Int16:
    case TypeId::Int32:
    case TypeId::Int64:
    case TypeId::UInt8:
    case TypeId::UInt16:
    case TypeId::UInt32:
    case TypeId::UInt64:
      return true;
    default:
      return false;
  }
}

auto int_is_in_range(int64_t value, Type& expected) -> bool {
  switch (expected.type_id) {
    case TypeId::Int8:
      return value >= INT8_MIN && value <= INT8_MAX;
    case TypeId::Int16:
      return value >= INT16_MIN && value <= INT16_MAX;
    case TypeId::Int32:
      return value >= INT32_MIN && value <= INT32_MAX;
    case TypeId::Int64:
      return value >= INT64_MIN && value <= INT64_MAX;
    case TypeId::UInt8:
      return value >= 0 && value <= UINT8_MAX;
    case TypeId::UInt16:
      return value >= 0 && value <= UINT16_MAX;
    case TypeId::UInt32:
      return value >= 0 && std::cmp_less_equal(value, UINT32_MAX);
    case TypeId::UInt64:
      return value >= 0;
    case TypeId::Float32: {
      constexpr int64_t MAX_F32_VALUE = 1LL << 24;
      return value >= -MAX_F32_VALUE && value <= MAX_F32_VALUE;
    }
    case TypeId::Float64: {
      constexpr int64_t MAX_F64_VALUE = 1LL << 53;
      return value >= -MAX_F64_VALUE && value <= MAX_F64_VALUE;
    }
    default:
      return false;
  }
}

auto is_float(Type& type) -> bool {
  switch (type.type_id) {
    case TypeId::Float32:
    case TypeId::Float64:
      return true;
    default:
      return false;
  }
}

auto float_is_in_range(double value, Type& expected) -> bool {
  switch (expected.type_id) {
    case TypeId::Float32: {
      constexpr auto MAX_F32_VALUE = static_cast<double>(1LL << 24);
      return value >= -MAX_F32_VALUE && value <= MAX_F32_VALUE;
    }
    case TypeId::Float64: {
      constexpr auto MAX_F64_VALUE = static_cast<double>(1LL << 53);
      return value >= -MAX_F64_VALUE && value <= MAX_F64_VALUE;
    }
    default:
      return false;
  }
}

auto get_literal_default(Type& type, SourceLocation loc) -> Type {
  switch (type.type_id) {
    case TypeId::IntLiteral:
      return Type{.type_id = TypeId::Int32, .identifier = "Int32"};
    case TypeId::FloatLiteral:
      return Type{.type_id = TypeId::Float64, .identifier = "Float64"};
    default:
      throw TypeError(
          loc, "Can't give a default literal value for non-literal type \"{}\"",
          type.identifier);
  }
}

auto TypeCheckVisitor::infer_expr_top_level(ASTNode* node) -> Type {
  auto resolved_type = infer_expr(node);

  // if it's a literal, we need to default it and push the type back down
  if (is_literal(resolved_type)) {
    resolved_type = get_literal_default(resolved_type, node->source_location);
    check_expr(node, resolved_type);
  }

  return resolved_type;
}

void TypeCheckVisitor::visit_statement_var_decl(
    VariableDeclarationStatement* decl) {
  auto it = variable_map.find(decl->name);

  if (it != variable_map.end()) {
    throw TypeError(decl->source_location,
                    "Attempted re-declaration of variable \"{}\"", decl->name);
  }

  // if there's a type annotation, drill it down into the value
  if (decl->type_identifier) {
    decl->declaration_type = infer_expr(decl->type_identifier.get());

    check_expr(decl->value.get(), *decl->declaration_type);
  } else {
    // otherwise infer it
    decl->declaration_type = infer_expr_top_level(decl->value.get());
  }

  variable_map[decl->name] = *decl->declaration_type;
}

void TypeCheckVisitor::visit_statement_if(IfStatement* ifstmt) {
  auto expected = BOOL_TYPE;
  check_expr(ifstmt->condition.get(), expected);

  for (auto& stmt : ifstmt->body) {
    visit_statement_node(stmt.get());
  }
}

void TypeCheckVisitor::visit_statement_var_assign(
    VariableAssignmentStatement* var) {
  auto it = variable_map.find(var->name);

  if (it == variable_map.end()) {
    throw TypeError(var->source_location, "Usage of undefined variable \"{}\"",
                    var->name);
  }

  auto& var_type = it->second;

  check_expr(var->value.get(), var_type);
}

void TypeCheckVisitor::visit_statement_program(Program* prog) {
  for (auto& fn : prog->functions) {
    visit_statement_node(fn.get());
    variable_map.clear();  // clear map between function declarations
  }
}

void TypeCheckVisitor::visit_statement_fn_decl(FunctionDeclaration* fn) {
  for (auto& stmt : fn->statements) {
    visit_statement_node(stmt.get());
  }

  // all functions return Int32 for now
  function_map[fn->name] =
      Type{.type_id = TypeId::Int32, .identifier = "Int32"};
}

void TypeCheckVisitor::visit_statement_show(ShowStatement* show) {
  show->expr_type = infer_expr_top_level(show->expr.get());
}

void TypeCheckVisitor::visit_statement_return(ReturnStatement* ret) {
  ret->expr_type = infer_expr_top_level(ret->expr.get());
}

/// Recursive visitor for statements. This is not for expressions.
void TypeCheckVisitor::visit_statement_node(ASTNode* node) {
  switch (node->kind()) {
    case NodeKind::FunctionDeclaration:
      visit_statement_fn_decl(cast<FunctionDeclaration>(node));
      break;
    case NodeKind::Program:
      visit_statement_program(cast<Program>(node));
      break;
    case NodeKind::VariableAssignmentStatement:
      visit_statement_var_assign(cast<VariableAssignmentStatement>(node));
      break;
    case NodeKind::VariableDeclarationStatement:
      visit_statement_var_decl(cast<VariableDeclarationStatement>(node));
      break;
    case NodeKind::ShowStatement:
      visit_statement_show(cast<ShowStatement>(node));
      break;
    case NodeKind::ReturnStatement:
      visit_statement_return(cast<ReturnStatement>(node));
      break;
    case NodeKind::IfStatement:
      visit_statement_if(cast<IfStatement>(node));
      break;
    case NodeKind::FloatLiteralExpression:
    case NodeKind::BinaryExpression:
    case NodeKind::VariableExpression:
    case NodeKind::FunctionCallExpression:
    case NodeKind::TypeExpression:
    case NodeKind::IntLiteralExpression:
      throw TypeError(
          node->source_location,
          "visit_statement_node should never visit an expression node");
  }
}

void TypeCheckVisitor::check_expr_int_literal(IntLiteralExpression* node,
                                              Type& expected) {
  if (is_integer(expected) || is_float(expected)) {
    if (!int_is_in_range(node->value, expected)) {
      throw TypeError(
          node->source_location,
          "Integer literal ({}) cannot fit in type {} without data loss",
          node->value, expected.identifier);
    }

    node->resolved_type = expected;
  } else {
    throw TypeError(node->source_location,
                    "Cannot implicitly convert Integer literal ({}) to type {}",
                    node->value, expected.identifier);
  }
}

void TypeCheckVisitor::check_expr_type_expr(TypeExpression* node,
                                            Type& expected) {
  throw TypeError(node->source_location,
                  "Attempt to check Type expression {} against type {}",
                  node->name, expected.identifier);
}

void TypeCheckVisitor::check_expr_func_call(FunctionCallExpression* node,
                                            Type& expected) {
  auto it = function_map.find(node->name);
  if (it == function_map.end()) {
    throw TypeError(node->source_location, "Call to undefined function \"{}\"",
                    node->name);
  }

  if (it->second.type_id != expected.type_id) {
    throw TypeError(node->source_location,
                    "Expected {}, Got {} from call to function \"{}\"",
                    expected.identifier, it->second.identifier, node->name);
  }
}

void TypeCheckVisitor::check_expr_var_expr(VariableExpression* node,
                                           Type& expected) {
  auto it = variable_map.find(node->name);
  if (it == variable_map.end()) {
    throw TypeError(node->source_location, "Usage of undefined variable \"{}\"",
                    node->name);
  }

  if (it->second.type_id != expected.type_id) {
    throw TypeError(node->source_location,
                    "Expected type {}, got {} from usage of variable \"{}\"",
                    expected.identifier, it->second.identifier, node->name);
  }

  node->resolved_type = expected;
}

void TypeCheckVisitor::check_expr_bin_expr(BinaryExpression* node,
                                           Type& expected) {
  // bool operators have a different operand type to resolved type
  Type left, right;
  if (is_bool_operator(node->op)) {
    node->resolved_type = BOOL_TYPE;

    // must infer concrete types since top level doesn't give us context to push
    // down
    left = infer_expr_top_level(node->lhs.get());
    right = infer_expr_top_level(node->rhs.get());

    if (left.type_id != right.type_id) {
      throw TypeError(node->source_location,
                      "Left type {} does not match right type {}",
                      left.identifier, right.identifier);
    }
  } else {
    node->resolved_type = expected;

    check_expr(node->lhs.get(), expected);
    check_expr(node->rhs.get(), expected);
  }
}

void TypeCheckVisitor::check_expr_float_literal(FloatLiteralExpression* node,
                                                Type& expected) {
  if (!is_float(expected)) {
    throw TypeError(node->source_location,
                    "Float literal ({}) cannot be converted to type {}",
                    node->value, expected.identifier);
  }

  if (!float_is_in_range(node->value, expected)) {
    throw TypeError(
        node->source_location,
        "Float literal ({}) cannot fit in type {} without data loss",
        node->value, expected.identifier);
  }

  node->resolved_type = expected;
}

void TypeCheckVisitor::check_expr(ASTNode* node, Type& expected) {
  switch (node->kind()) {
    case NodeKind::IntLiteralExpression:
      check_expr_int_literal(cast<IntLiteralExpression>(node), expected);
      break;
    case NodeKind::TypeExpression:
      check_expr_type_expr(cast<TypeExpression>(node), expected);
      break;
    case NodeKind::FunctionCallExpression:
      check_expr_func_call(cast<FunctionCallExpression>(node), expected);
      break;
    case NodeKind::VariableExpression:
      check_expr_var_expr(cast<VariableExpression>(node), expected);
      break;
    case NodeKind::BinaryExpression:
      check_expr_bin_expr(cast<BinaryExpression>(node), expected);
      break;
    case NodeKind::FloatLiteralExpression:
      check_expr_float_literal(cast<FloatLiteralExpression>(node), expected);
      break;

    case NodeKind::IfStatement:
    case NodeKind::ReturnStatement:
    case NodeKind::ShowStatement:
    case NodeKind::VariableDeclarationStatement:
    case NodeKind::VariableAssignmentStatement:
    case NodeKind::Program:
    case NodeKind::FunctionDeclaration:
      throw TypeError(node->source_location,
                      "Shouldn't ever call check on a statement...");
  }
}

auto TypeCheckVisitor::infer_expr_int_literal(IntLiteralExpression* node)
    -> Type {
  node->resolved_type =
      Type{.type_id = TypeId::IntLiteral, .identifier = "IntLiteral"};
  return *node->resolved_type;
}

auto TypeCheckVisitor::infer_expr_type_expr(TypeExpression* node) -> Type {
  if (!is_builtin_type(node->name)) {
    throw TypeError(node->source_location, "User type {} is invalid",
                    node->name);
  }

  return Type{.type_id = get_type_id(node->name), .identifier = node->name};
}

auto TypeCheckVisitor::infer_expr_func_call(FunctionCallExpression* node)
    -> Type {
  auto it = function_map.find(node->name);
  if (it == function_map.end()) {
    throw TypeError(node->source_location, "Call to undefined function \"{}\"",
                    node->name);
  }

  return it->second;
}

auto TypeCheckVisitor::infer_expr_var_expr(VariableExpression* node) -> Type {
  auto it = variable_map.find(node->name);
  if (it == variable_map.end()) {
    throw TypeError(node->source_location, "Usage of undefined variable \"{}\"",
                    node->name);
  }

  node->resolved_type = it->second;
  return *node->resolved_type;
}

auto TypeCheckVisitor::infer_expr_bin_expr(BinaryExpression* node) -> Type {
  auto left = infer_expr(node->lhs.get());
  auto right = infer_expr(node->rhs.get());

  if (is_bool_operator(node->op)) {
    node->resolved_type = BOOL_TYPE;
  }

  auto left_is_literal = is_literal(left);
  auto right_is_literal = is_literal(right);

  if (left_is_literal && right_is_literal) {
    auto left_is_int = left.type_id == TypeId::IntLiteral;
    auto right_is_int = right.type_id == TypeId::IntLiteral;

    if (left_is_int && right_is_int) {
      node->operand_type = INT_LITERAL_TYPE;
      return INT_LITERAL_TYPE;
    }

    // if one is a float, default both to float
    node->operand_type = FLOAT_LITERAL_TYPE;
    return FLOAT_LITERAL_TYPE;
  }

  // only one is a literal, the other is concrete so we can infer from context
  if (left_is_literal != right_is_literal) {
    Pair pair = left_is_literal ? Pair{.from = left, .to = right}
                                : Pair{.from = right, .to = left};

    auto it = LITERAL_PROMOTION_MAP.find(pair);
    if (it == LITERAL_PROMOTION_MAP.end()) {
      throw TypeError(
          node->source_location,
          "Unable to automatically convert literal {} to target type {}",
          pair.from.identifier, pair.to.identifier);
    }

    node->operand_type = it->second;
  }

  if (left.type_id != right.type_id) {
    throw TypeError(node->source_location,
                    "Left type {} does not match right type {}",
                    left.identifier, right.identifier);
  }

  if (!node->resolved_type) node->resolved_type = node->operand_type;

  return *node->resolved_type;
}

auto TypeCheckVisitor::infer_expr_float_literal(FloatLiteralExpression* node)
    -> Type {
  return Type{.type_id = TypeId::FloatLiteral, .identifier = "FloatLiteral"};
}

auto TypeCheckVisitor::infer_expr(ASTNode* node) -> Type {
  switch (node->kind()) {
    case NodeKind::IntLiteralExpression:
      return infer_expr_int_literal(cast<IntLiteralExpression>(node));
    case NodeKind::TypeExpression:
      return infer_expr_type_expr(cast<TypeExpression>(node));
    case NodeKind::FunctionCallExpression:
      return infer_expr_func_call(cast<FunctionCallExpression>(node));
    case NodeKind::VariableExpression:
      return infer_expr_var_expr(cast<VariableExpression>(node));
    case NodeKind::BinaryExpression:
      return infer_expr_bin_expr(cast<BinaryExpression>(node));
    case NodeKind::FloatLiteralExpression:
      return infer_expr_float_literal(cast<FloatLiteralExpression>(node));

    case NodeKind::IfStatement:
    case NodeKind::ReturnStatement:
    case NodeKind::ShowStatement:
    case NodeKind::VariableDeclarationStatement:
    case NodeKind::VariableAssignmentStatement:
    case NodeKind::Program:
    case NodeKind::FunctionDeclaration:
      throw TypeError(node->source_location,
                      "Shouldn't ever call check on a statement...");
  }
}
