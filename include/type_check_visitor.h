#ifndef TYPE_CHECK_VISITOR_H
#define TYPE_CHECK_VISITOR_H

#include "ast.h"

auto check(VariableDeclarationStatement& stmt) -> Type;

class VariableScopeStack {
 public:
  VariableScopeStack();
  auto find(const std::string& key) -> std::optional<Type>;
  auto store(const std::string& key, Type& value) -> void;
  auto push() -> void;
  auto pop() -> void;

 private:
  std::vector<std::unordered_map<std::string, Type>> _scope_map;
};

struct Pair {
  Type left;
  Type right;

  auto operator==(const Pair& pair) const -> bool {
    auto left = pair.left.type_id == this->left.type_id;
    auto right = pair.right.type_id == this->right.type_id;

    return right && left;
  }
};

namespace std {
template <>
struct hash<Pair> {
  auto operator()(const Pair& k) const noexcept -> std::size_t {
    std::size_t h1 = std::hash<TypeId>{}(k.left.type_id);
    std::size_t h2 = std::hash<TypeId>{}(k.right.type_id);

    return h1 ^ (h2 << 1);
  }
};
};  // namespace std

const std::unordered_map<Pair, Type> LITERAL_PROMOTION_MAP{
    {Pair{.left = INT_LITERAL_TYPE, .right = INT8_TYPE}, INT8_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = INT16_TYPE}, INT16_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = INT32_TYPE}, INT32_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = INT64_TYPE}, INT64_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = UINT8_TYPE}, UINT8_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = UINT16_TYPE}, UINT16_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = UINT32_TYPE}, UINT32_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = UINT64_TYPE}, UINT64_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = FLOAT32_TYPE}, FLOAT32_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = FLOAT64_TYPE}, FLOAT64_TYPE},
    {Pair{.left = FLOAT_LITERAL_TYPE, .right = FLOAT32_TYPE}, FLOAT32_TYPE},
    {Pair{.left = FLOAT_LITERAL_TYPE, .right = FLOAT64_TYPE}, FLOAT64_TYPE},
    {Pair{.left = INT8_TYPE, .right = INT_LITERAL_TYPE}, INT8_TYPE},
    {Pair{.left = INT16_TYPE, .right = INT_LITERAL_TYPE}, INT16_TYPE},
    {Pair{.left = INT32_TYPE, .right = INT_LITERAL_TYPE}, INT32_TYPE},
    {Pair{.left = INT64_TYPE, .right = INT_LITERAL_TYPE}, INT64_TYPE},
    {Pair{.left = UINT8_TYPE, .right = INT_LITERAL_TYPE}, UINT8_TYPE},
    {Pair{.left = UINT16_TYPE, .right = INT_LITERAL_TYPE}, UINT16_TYPE},
    {Pair{.left = UINT32_TYPE, .right = INT_LITERAL_TYPE}, UINT32_TYPE},
    {Pair{.left = UINT64_TYPE, .right = INT_LITERAL_TYPE}, UINT64_TYPE},
    {Pair{.left = FLOAT32_TYPE, .right = INT_LITERAL_TYPE}, FLOAT32_TYPE},
    {Pair{.left = FLOAT64_TYPE, .right = INT_LITERAL_TYPE}, FLOAT64_TYPE},
    {Pair{.left = FLOAT32_TYPE, .right = FLOAT_LITERAL_TYPE}, FLOAT32_TYPE},
    {Pair{.left = FLOAT64_TYPE, .right = FLOAT_LITERAL_TYPE}, FLOAT64_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = FLOAT_LITERAL_TYPE},
     FLOAT_LITERAL_TYPE},
    {Pair{.left = FLOAT_LITERAL_TYPE, .right = INT_LITERAL_TYPE},
     FLOAT_LITERAL_TYPE},
    {Pair{.left = INT_LITERAL_TYPE, .right = INT_LITERAL_TYPE},
     INT_LITERAL_TYPE},
    {Pair{.left = FLOAT_LITERAL_TYPE, .right = FLOAT_LITERAL_TYPE},
     FLOAT_LITERAL_TYPE},
};

class TypeCheckVisitor {
 public:
  TypeCheckVisitor() = default;
  // void finalise();

  void visit_statement_node(ASTNode* node);
  void visit_statement_program(Program* prog);
  void visit_statement_fn_decl(FunctionDeclaration* fn);
  void visit_statement_var_decl(VariableDeclarationStatement* decl);
  void visit_statement_if(IfStatement* ifstmt);
  void visit_statement_var_assign(VariableAssignmentStatement* var);
  void visit_statement_show(ShowStatement* show);
  void visit_statement_return(ReturnStatement* ret);

  void check_expr(ASTNode* node, Type& expected);
  void check_expr_int_literal(IntLiteralExpression* node, Type& expected);
  void check_expr_type_expr(TypeExpression* node, Type& expected);
  void check_expr_func_call(FunctionCallExpression* node, Type& expected);
  void check_expr_var_expr(VariableExpression* node, Type& expected);
  void check_expr_bin_expr(BinaryExpression* node, Type& expected);
  void check_expr_float_literal(FloatLiteralExpression* node, Type& expected);

  auto infer_expr(ASTNode* node) -> Type;
  auto infer_expr_int_literal(IntLiteralExpression* node) -> Type;
  auto infer_expr_type_expr(TypeExpression* node) -> Type;
  auto infer_expr_func_call(FunctionCallExpression* node) -> Type;
  auto infer_expr_var_expr(VariableExpression* node) -> Type;
  auto infer_expr_bin_expr(BinaryExpression* node) -> Type;
  auto infer_expr_float_literal(FloatLiteralExpression* node) -> Type;

  /// Always resolves to non-literal concrete type
  /// by defaulting literals and pushing them back down.
  /// This is basically the "YOU NEED TO RESOLVE NOW" wrapper.
  auto infer_expr_top_level(ASTNode* node) -> Type;

  Type resolve_literal_pair(const BinaryExpression* bin, Type& left,
                            Type& right);

  Type result;
  VariableScopeStack scope_stack;
  std::unordered_map<std::string, Type> function_map;
  FunctionDeclaration* current_function{};
};

#endif  // TYPE_CHECK_VISITOR_H
