#ifndef TYPE_CHECK_VISITOR_H
#define TYPE_CHECK_VISITOR_H

#include "utils/container.h"
#include "visitor.h"

auto check(VariableDeclarationStatement& stmt) -> Type;

struct Pair {
  Type from;
  Type to;

  auto operator==(const Pair& pair) const -> bool {
    auto from = pair.from.type_id == this->from.type_id;
    auto to = pair.to.type_id == this->to.type_id;

    return to && from;
  }
};

namespace std {
template <>
struct hash<Pair> {
  auto operator()(const Pair& k) const noexcept -> std::size_t {
    // Compute individual hashes
    std::size_t h1 = std::hash<TypeId>{}(k.from.type_id);
    std::size_t h2 = std::hash<TypeId>{}(k.to.type_id);

    // Combine hashes using a standard bit-shifting formula
    return h1 ^ (h2 << 1);
  }
};
};  // namespace std

const std::unordered_map<Pair, Type> LITERAL_PROMOTION_MAP{
    {Pair{.from = INT_LITERAL_TYPE, .to = INT8_TYPE}, INT8_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = INT16_TYPE}, INT16_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = INT32_TYPE}, INT32_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = INT64_TYPE}, INT64_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = UINT8_TYPE}, UINT8_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = UINT16_TYPE}, UINT16_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = UINT32_TYPE}, UINT32_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = UINT64_TYPE}, UINT64_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = FLOAT32_TYPE}, FLOAT32_TYPE},
    {Pair{.from = INT_LITERAL_TYPE, .to = FLOAT64_TYPE}, FLOAT64_TYPE},
    {Pair{.from = FLOAT_LITERAL_TYPE, .to = FLOAT32_TYPE}, FLOAT32_TYPE},
    {Pair{.from = FLOAT_LITERAL_TYPE, .to = FLOAT64_TYPE}, FLOAT64_TYPE},
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

  Type result;
  std::unordered_map<std::string, Type> variable_map;
  std::unordered_map<std::string, Type> function_map;
  FunctionDeclaration* current_function{};
};

#endif  // TYPE_CHECK_VISITOR_H
