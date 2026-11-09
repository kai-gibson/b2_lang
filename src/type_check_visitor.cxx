#include "type_check_visitor.h"

#include <utility>

#include "ast.h"
#include "compile_error.h"
#include "lexer.h"
#include "parser.h"

auto is_literal(const std::unique_ptr<Type>& type) -> bool {
  switch (type->kind()) {
    case TypeKind::IntLiteral:
    case TypeKind::FloatLiteral:
      return true;
    default:
      return false;
  }
}

auto is_integer(const std::unique_ptr<Type>& type) -> bool {
  switch (type->kind()) {
    case TypeKind::Int8:
    case TypeKind::Int16:
    case TypeKind::Int32:
    case TypeKind::Int64:
    case TypeKind::UInt8:
    case TypeKind::UInt16:
    case TypeKind::UInt32:
    case TypeKind::UInt64:
      return true;
    default:
      return false;
  }
}

auto int_is_in_range(mp::int128_t& value, std::unique_ptr<Type>& expected)
    -> bool {
  switch (expected->kind()) {
    case TypeKind::Int8:
      return value >= INT8_MIN && value <= INT8_MAX;
    case TypeKind::Int16:
      return value >= INT16_MIN && value <= INT16_MAX;
    case TypeKind::Int32:
      return value >= INT32_MIN && value <= INT32_MAX;
    case TypeKind::Int64:
      return value >= INT64_MIN && value <= INT64_MAX;
    case TypeKind::UInt8:
      return value >= 0 && value <= UINT8_MAX;
    case TypeKind::UInt16:
      return value >= 0 && value <= UINT16_MAX;
    case TypeKind::UInt32:
      return value >= 0 && value <= UINT32_MAX;
    case TypeKind::UInt64:
      return value >= 0 && value <= UINT64_MAX;
    case TypeKind::Float32: {
      constexpr int64_t MAX_F32_VALUE = 1LL << 24;
      return value >= -MAX_F32_VALUE && value <= MAX_F32_VALUE;
    }
    case TypeKind::Float64: {
      constexpr int64_t MAX_F64_VALUE = 1LL << 53;
      return value >= -MAX_F64_VALUE && value <= MAX_F64_VALUE;
    }
    default:
      return false;
  }
}

auto is_float(std::unique_ptr<Type>& type) -> bool {
  switch (type->kind()) {
    case TypeKind::Float32:
    case TypeKind::Float64:
      return true;
    default:
      return false;
  }
}

auto float_is_in_range(double value, std::unique_ptr<Type>& expected) -> bool {
  switch (expected->kind()) {
    case TypeKind::Float32: {
      constexpr auto MAX_F32_VALUE = static_cast<double>(1LL << 24);
      return value >= -MAX_F32_VALUE && value <= MAX_F32_VALUE;
    }
    case TypeKind::Float64: {
      constexpr auto MAX_F64_VALUE = static_cast<double>(1LL << 53);
      return value >= -MAX_F64_VALUE && value <= MAX_F64_VALUE;
    }
    default:
      return false;
  }
}

auto get_literal_default(std::unique_ptr<Type>& type, SourceLocation loc)
    -> std::unique_ptr<Type> {
  switch (type->kind()) {
    case TypeKind::IntLiteral:
      return std::make_unique<Int32Type>();
    case TypeKind::FloatLiteral:
      return std::make_unique<Float64Type>();
    default:
      throw TypeError(
          loc, "Can't give a default literal value for non-literal type \"{}\"",
          type->kind_name());
  }
}

VariableScopeStack::VariableScopeStack() { _scope_map.emplace_back(); }

auto VariableScopeStack::find(const std::string& key)
    -> std::unique_ptr<Type>* {
  for (auto& scope : _scope_map) {
    auto it = scope.find(key);
    if (it != scope.end()) return &it->second;
  }

  return nullptr;
}

auto VariableScopeStack::push() -> void { _scope_map.emplace_back(); }
auto VariableScopeStack::pop() -> void { _scope_map.pop_back(); }
auto VariableScopeStack::store(const std::string& key,
                               std::unique_ptr<Type> value) -> void {
  _scope_map.back()[key] = std::move(value);
}

auto TypeCheckVisitor::infer_expr_top_level(ASTNode* node)
    -> std::unique_ptr<Type> {
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
  auto found = variable_scope_stack.find(decl->name);

  if (found) {
    throw TypeError(decl->source_location,
                    "Attempted re-declaration of variable \"{}\"", decl->name);
  }

  // if there's a type annotation, drill it down into the value
  if (decl->type_identifier) {
    decl->declaration_type = infer_expr(decl->type_identifier.get());

    check_expr(decl->value.get(), decl->declaration_type);
  } else {
    // otherwise infer it
    decl->declaration_type = infer_expr_top_level(decl->value.get());
  }

  variable_scope_stack.store(decl->name, decl->declaration_type->clone());
}

void TypeCheckVisitor::visit_statement_if(IfStatement* ifstmt) {
  std::unique_ptr<Type> expected = std::make_unique<BoolType>();

  check_expr(ifstmt->condition.get(), expected);

  visit_statement_node(ifstmt->if_then.get());

  if (ifstmt->if_else) {
    visit_statement_node(ifstmt->if_else.get());
  }
}

void TypeCheckVisitor::visit_statement_block_stmt(BlockStatement* block) {
  variable_scope_stack.push();

  for (const auto& stmt : block->statements) {
    visit_statement_node(stmt.get());
  }

  variable_scope_stack.pop();
}

void TypeCheckVisitor::visit_statement_var_assign(
    VariableAssignmentStatement* var) {
  auto found = this->variable_scope_stack.find(var->name);

  if (found == nullptr) {
    throw TypeError(var->source_location, "Usage of undefined variable \"{}\"",
                    var->name);
  }

  auto& var_type = *found;

  check_expr(var->value.get(), var_type);
}

void TypeCheckVisitor::visit_statement_program(Program* prog) {
  for (auto& fn : prog->functions) {
    visit_statement_node(fn.get());
    variable_scope_stack.pop();
  }
}

void TypeCheckVisitor::visit_statement_fn_decl(FunctionDeclaration* fn) {
  visit_statement_node(fn->statements.get());

  // all functions return Int32 for now
  function_map[fn->name] = std::make_unique<Int32Type>();
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
    case NodeKind::BlockStatement:
      visit_statement_block_stmt(cast<BlockStatement>(node));
      break;
    case NodeKind::LoopStatement:
      visit_statement_loop(cast<LoopStatement>(node));
      break;
    case NodeKind::BreakStatement:
      visit_statement_break(cast<BreakStatement>(node));
      break;
    case NodeKind::CycleStatement:
      visit_statement_cycle(cast<CycleStatement>(node));
      break;
    case NodeKind::FloatLiteralExpression:
    case NodeKind::BinaryExpression:
    case NodeKind::VariableExpression:
    case NodeKind::FunctionCallExpression:
    case NodeKind::TypeExpression:
    case NodeKind::PointerTypeExpression:
    case NodeKind::IntLiteralExpression:
      throw TypeError(
          node->source_location,
          "visit_statement_node should never visit an expression node");
  }
}

void TypeCheckVisitor::check_expr_int_literal(IntLiteralExpression* node,
                                              std::unique_ptr<Type>& expected) {
  if (is_integer(expected) || is_float(expected)) {
    if (!int_is_in_range(node->value, expected)) {
      throw TypeError(
          node->source_location,
          "Integer literal ({}) cannot fit in type {} without data loss",
          node->value.str(), expected->kind_name());
    }

    node->resolved_type = expected->clone();
  } else {
    throw TypeError(node->source_location,
                    "Cannot implicitly convert Integer literal ({}) to type {}",
                    node->value.str(), expected->kind_name());
  }
}

void TypeCheckVisitor::check_expr_type_expr(TypeExpression* node,
                                            std::unique_ptr<Type>& expected) {
  throw TypeError(node->source_location,
                  "Attempt to check Type expression {} against type {}",
                  node->name, expected->kind_name());
}

void TypeCheckVisitor::check_expr_pointer_type_expr(
    PointerTypeExpression* node, std::unique_ptr<Type>& expected) {
  throw TypeError(node->source_location,
                  "Attempt to check PointerType expression against type {}",
                  expected->kind_name());
}

void TypeCheckVisitor::check_expr_func_call(FunctionCallExpression* node,
                                            std::unique_ptr<Type>& expected) {
  auto it = function_map.find(node->name);
  if (it == function_map.end()) {
    throw TypeError(node->source_location, "Call to undefined function \"{}\"",
                    node->name);
  }

  if (it->second->kind() != expected->kind()) {
    throw TypeError(node->source_location,
                    "Expected {}, Got {} from call to function \"{}\"",
                    expected->kind_name(), it->second->kind_name(), node->name);
  }
}

void TypeCheckVisitor::check_expr_var_expr(VariableExpression* node,
                                           std::unique_ptr<Type>& expected) {
  auto found = variable_scope_stack.find(node->name);
  if (found == nullptr) {
    throw TypeError(node->source_location, "Usage of undefined variable \"{}\"",
                    node->name);
  }

  if ((*found)->kind() != expected->kind()) {
    throw TypeError(node->source_location,
                    "Expected type {}, got {} from usage of variable \"{}\"",
                    expected->kind_name(), (*found)->kind_name(), node->name);
  }

  node->resolved_type = expected->clone();
}

void TypeCheckVisitor::check_expr_bin_expr(BinaryExpression* node,
                                           std::unique_ptr<Type>& expected) {
  // bool operators have a different operand type to resolved type
  if (is_bool_operator(node->op)) {
    // bools get inferred since their resolved type isn't related to the
    // operand type
    infer_expr_bin_expr(node);
  } else {
    node->resolved_type = expected->clone();
    node->operand_type = expected->clone();

    check_expr(node->lhs.get(), expected);
    check_expr(node->rhs.get(), expected);
  }
}

void TypeCheckVisitor::check_expr_float_literal(
    FloatLiteralExpression* node, std::unique_ptr<Type>& expected) {
  if (!is_float(expected)) {
    throw TypeError(node->source_location,
                    "Float literal ({}) cannot be converted to type {}",
                    node->value, expected->kind_name());
  }

  if (!float_is_in_range(node->value, expected)) {
    throw TypeError(
        node->source_location,
        "Float literal ({}) cannot fit in type {} without data loss",
        node->value, expected->kind_name());
  }

  node->resolved_type = expected->clone();
}

void TypeCheckVisitor::check_expr(ASTNode* node,
                                  std::unique_ptr<Type>& expected) {
  switch (node->kind()) {
    case NodeKind::IntLiteralExpression:
      check_expr_int_literal(cast<IntLiteralExpression>(node), expected);
      break;
    case NodeKind::TypeExpression:
      check_expr_type_expr(cast<TypeExpression>(node), expected);
      break;
    case NodeKind::PointerTypeExpression:
      check_expr_pointer_type_expr(cast<PointerTypeExpression>(node), expected);
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
    case NodeKind::CycleStatement:
      [[fallthrough]];
    case NodeKind::BreakStatement:
      [[fallthrough]];
    case NodeKind::LoopStatement:
      [[fallthrough]];
    case NodeKind::BlockStatement:
      [[fallthrough]];
    case NodeKind::IfStatement:
      [[fallthrough]];
    case NodeKind::ReturnStatement:
      [[fallthrough]];
    case NodeKind::ShowStatement:
      [[fallthrough]];
    case NodeKind::VariableDeclarationStatement:
      [[fallthrough]];
    case NodeKind::VariableAssignmentStatement:
      [[fallthrough]];
    case NodeKind::Program:
      [[fallthrough]];
    case NodeKind::FunctionDeclaration:
      throw TypeError(node->source_location,
                      "Shouldn't ever call check on a statement...");
  }
}

auto TypeCheckVisitor::infer_expr_int_literal(IntLiteralExpression* node)
    -> std::unique_ptr<Type> {
  node->resolved_type = std::make_unique<IntLiteralType>();
  return node->resolved_type->clone();
}

auto TypeCheckVisitor::infer_expr_type_expr(TypeExpression* node)
    -> std::unique_ptr<Type> {
  if (!is_builtin_type(node->name)) {
    throw TypeError(node->source_location, "Type {} is invalid", node->name);
  }

  return str_to_type(node->name);
}

auto TypeCheckVisitor::infer_expr_pointer_type_expr(PointerTypeExpression* node)
    -> std::unique_ptr<Type> {
  auto pointer_type = std::make_unique<PointerType>();
  pointer_type->base = infer_expr(node->base_type.get());

  return pointer_type;
}

auto TypeCheckVisitor::infer_expr_func_call(FunctionCallExpression* node)
    -> std::unique_ptr<Type> {
  auto it = function_map.find(node->name);
  if (it == function_map.end()) {
    throw TypeError(node->source_location, "Call to undefined function \"{}\"",
                    node->name);
  }

  return it->second->clone();
}

auto TypeCheckVisitor::infer_expr_var_expr(VariableExpression* node)
    -> std::unique_ptr<Type> {
  auto found = variable_scope_stack.find(node->name);
  if (found == nullptr) {
    throw TypeError(node->source_location, "Usage of undefined variable \"{}\"",
                    node->name);
  }

  node->resolved_type = (*found)->clone();
  return node->resolved_type->clone();
}

/*
If a literal is involved, resolve to either concrete type or the best matching
literal
 */
auto TypeCheckVisitor::resolve_literal_pair(const BinaryExpression* bin,
                                            std::unique_ptr<Type>& left,
                                            std::unique_ptr<Type>& right)
    -> std::unique_ptr<Type> {
  auto it = LITERAL_PROMOTION_MAP.find(
      Pair{.left = left->kind(), .right = right->kind()});
  if (it == LITERAL_PROMOTION_MAP.end()) {
    throw TypeError(
        bin->source_location,
        "Left {} and right {} types are not compatible for operation {}",
        left->kind_name(), right->kind_name(), token_type_to_str(bin->op));
  }

  auto found = type_factory(it->second);

  // push concrete type down if one is found
  if (!is_literal(found)) {
    if (is_literal(left)) {
      check_expr(bin->lhs.get(), found);
    } else {
      check_expr(bin->rhs.get(), found);
    }
  }

  left = found->clone();
  right = found->clone();
  return found;
}

auto TypeCheckVisitor::infer_expr_bin_expr(BinaryExpression* node)
    -> std::unique_ptr<Type> {
  auto left = infer_expr(node->lhs.get());
  auto right = infer_expr(node->rhs.get());

  std::unique_ptr<Type> operand_type, resolved_type;

  if (is_literal(left) || is_literal(right)) {
    operand_type = resolve_literal_pair(node, left, right);
  }

  if (left->kind() != right->kind()) {
    throw TypeError(node->source_location,
                    "Left {} and right {} types do not match for operation {}",
                    left->kind_name(), right->kind_name(),
                    token_type_to_str(node->op));
  }

  if (is_bool_operator(node->op)) {
    resolved_type = std::make_unique<BoolType>();

    // bool must resolve now - no further context above
    if (is_literal(left) && is_literal(right)) {
      operand_type = get_literal_default(left, node->source_location);

      check_expr(node->lhs.get(), operand_type);
      check_expr(node->rhs.get(), operand_type);
    } else {
      operand_type = left->clone();
    }

  } else {
    if (operand_type == nullptr) {
      operand_type = left->clone();
    }
    resolved_type = operand_type->clone();
  }

  node->operand_type = std::move(operand_type);
  node->resolved_type = resolved_type->clone();
  return resolved_type;
}

auto TypeCheckVisitor::infer_expr_float_literal(FloatLiteralExpression* node)
    -> std::unique_ptr<Type> {
  return std::make_unique<FloatLiteralType>();
}

auto TypeCheckVisitor::infer_expr(ASTNode* node) -> std::unique_ptr<Type> {
  switch (node->kind()) {
    case NodeKind::IntLiteralExpression:
      return infer_expr_int_literal(cast<IntLiteralExpression>(node));
    case NodeKind::TypeExpression:
      return infer_expr_type_expr(cast<TypeExpression>(node));
    case NodeKind::PointerTypeExpression:
      return infer_expr_pointer_type_expr(cast<PointerTypeExpression>(node));
    case NodeKind::FunctionCallExpression:
      return infer_expr_func_call(cast<FunctionCallExpression>(node));
    case NodeKind::VariableExpression:
      return infer_expr_var_expr(cast<VariableExpression>(node));
    case NodeKind::BinaryExpression:
      return infer_expr_bin_expr(cast<BinaryExpression>(node));
    case NodeKind::FloatLiteralExpression:
      return infer_expr_float_literal(cast<FloatLiteralExpression>(node));

    case NodeKind::CycleStatement:
      [[fallthrough]];
    case NodeKind::BreakStatement:
      [[fallthrough]];
    case NodeKind::LoopStatement:
      [[fallthrough]];
    case NodeKind::IfStatement:
      [[fallthrough]];
    case NodeKind::BlockStatement:
      [[fallthrough]];
    case NodeKind::ReturnStatement:
      [[fallthrough]];
    case NodeKind::ShowStatement:
      [[fallthrough]];
    case NodeKind::VariableDeclarationStatement:
      [[fallthrough]];
    case NodeKind::VariableAssignmentStatement:
      [[fallthrough]];
    case NodeKind::Program:
      [[fallthrough]];
    case NodeKind::FunctionDeclaration:
      throw TypeError(node->source_location,
                      "Shouldn't ever call check on a statement...");
  }
}

void TypeCheckVisitor::visit_statement_loop(LoopStatement* loop) {
  loop_depth += 1;

  visit_statement_node(loop->body.get());

  loop_depth -= 1;
}

void TypeCheckVisitor::visit_statement_break(BreakStatement* brk) {
  if (loop_depth == 0) {
    throw TypeError(brk->source_location,
                    "Break statement cannot appear outside a loop");
  }
}

void TypeCheckVisitor::visit_statement_cycle(CycleStatement* cycle) {
  if (loop_depth == 0) {
    throw TypeError(cycle->source_location,
                    "Cycle statement cannot appear outside a loop");
  }
}
