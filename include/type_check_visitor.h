#ifndef TYPE_CHECK_VISITOR_H
#define TYPE_CHECK_VISITOR_H

#include "ast.h"
#include "types.h"

class VariableScopeStack {
 public:
  VariableScopeStack();
  auto find(const std::string& key) -> std::unique_ptr<Type>*;
  auto store(const std::string& key, std::unique_ptr<Type> value) -> void;
  auto push() -> void;
  auto pop() -> void;

 private:
  std::vector<std::unordered_map<std::string, std::unique_ptr<Type>>>
      _scope_map;
};

struct Pair {
  TypeKind left;
  TypeKind right;

  auto operator==(const Pair& pair) const -> bool {
    auto left = pair.left == this->left;
    auto right = pair.right == this->right;

    return right && left;
  }
};

namespace std {
template <>
struct hash<Pair> {
  auto operator()(const Pair& k) const noexcept -> std::size_t {
    std::size_t h1 = std::hash<TypeKind>{}(k.left);
    std::size_t h2 = std::hash<TypeKind>{}(k.right);

    return h1 ^ (h2 << 1);
  }
};
};  // namespace std

const std::unordered_map<Pair, TypeKind> LITERAL_PROMOTION_MAP{
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::Int8},
     TypeKind::Int8},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::Int16},
     TypeKind::Int16},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::Int32},
     TypeKind::Int32},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::Int64},
     TypeKind::Int64},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::UInt8},
     TypeKind::UInt8},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::UInt16},
     TypeKind::UInt16},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::UInt32},
     TypeKind::UInt32},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::UInt64},
     TypeKind::UInt64},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::Float32},
     TypeKind::Float32},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::Float64},
     TypeKind::Float64},
    {Pair{.left = TypeKind::FloatLiteral, .right = TypeKind::Float32},
     TypeKind::Float32},
    {Pair{.left = TypeKind::FloatLiteral, .right = TypeKind::Float64},
     TypeKind::Float64},
    {Pair{.left = TypeKind::Int8, .right = TypeKind::IntLiteral},
     TypeKind::Int8},
    {Pair{.left = TypeKind::Int16, .right = TypeKind::IntLiteral},
     TypeKind::Int16},
    {Pair{.left = TypeKind::Int32, .right = TypeKind::IntLiteral},
     TypeKind::Int32},
    {Pair{.left = TypeKind::Int64, .right = TypeKind::IntLiteral},
     TypeKind::Int64},
    {Pair{.left = TypeKind::UInt8, .right = TypeKind::IntLiteral},
     TypeKind::UInt8},
    {Pair{.left = TypeKind::UInt16, .right = TypeKind::IntLiteral},
     TypeKind::UInt16},
    {Pair{.left = TypeKind::UInt32, .right = TypeKind::IntLiteral},
     TypeKind::UInt32},
    {Pair{.left = TypeKind::UInt64, .right = TypeKind::IntLiteral},
     TypeKind::UInt64},
    {Pair{.left = TypeKind::Float32, .right = TypeKind::IntLiteral},
     TypeKind::Float32},
    {Pair{.left = TypeKind::Float64, .right = TypeKind::IntLiteral},
     TypeKind::Float64},
    {Pair{.left = TypeKind::Float32, .right = TypeKind::FloatLiteral},
     TypeKind::Float32},
    {Pair{.left = TypeKind::Float64, .right = TypeKind::FloatLiteral},
     TypeKind::Float64},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::FloatLiteral},
     TypeKind::FloatLiteral},
    {Pair{.left = TypeKind::FloatLiteral, .right = TypeKind::IntLiteral},
     TypeKind::FloatLiteral},
    {Pair{.left = TypeKind::IntLiteral, .right = TypeKind::IntLiteral},
     TypeKind::IntLiteral},
    {Pair{.left = TypeKind::FloatLiteral, .right = TypeKind::FloatLiteral},
     TypeKind::FloatLiteral},
};

class TypeCheckVisitor {
 public:
  // TypeCheckVisitor() = default;
  //  void finalise();

  void visit_statement_node(ASTNode* node);
  void visit_statement_program(Program* prog);
  void visit_statement_fn_decl(FunctionDeclaration* fn);
  void visit_statement_var_decl(VariableDeclarationStatement* decl);
  void visit_statement_if(IfStatement* ifstmt);
  void visit_statement_block_stmt(BlockStatement* elseifstmt);
  void visit_statement_var_assign(VariableAssignmentStatement* var);
  void visit_statement_show(ShowStatement* show);
  void visit_statement_return(ReturnStatement* ret);
  void visit_statement_loop(LoopStatement* loop);
  void visit_statement_break(BreakStatement* brk);
  void visit_statement_cycle(CycleStatement* cycle);

  void check_expr(ASTNode* node, std::unique_ptr<Type>& expected);
  void check_expr_int_literal(IntLiteralExpression* node,
                              std::unique_ptr<Type>& expected);
  void check_expr_type_expr(TypeExpression* node,
                            std::unique_ptr<Type>& expected);
  void check_expr_pointer_type_expr(PointerTypeExpression* node,
                                    std::unique_ptr<Type>& expected);
  void check_expr_func_call(FunctionCallExpression* node,
                            std::unique_ptr<Type>& expected);
  void check_expr_var_expr(VariableExpression* node,
                           std::unique_ptr<Type>& expected);
  void check_expr_bin_expr(BinaryExpression* node,
                           std::unique_ptr<Type>& expected);
  void check_expr_float_literal(FloatLiteralExpression* node,
                                std::unique_ptr<Type>& expected);

  auto infer_expr(ASTNode* node) -> std::unique_ptr<Type>;
  auto infer_expr_int_literal(IntLiteralExpression* node)
      -> std::unique_ptr<Type>;
  auto infer_expr_type_expr(TypeExpression* node) -> std::unique_ptr<Type>;
  auto infer_expr_pointer_type_expr(PointerTypeExpression* node)
      -> std::unique_ptr<Type>;
  auto infer_expr_func_call(FunctionCallExpression* node)
      -> std::unique_ptr<Type>;
  auto infer_expr_var_expr(VariableExpression* node) -> std::unique_ptr<Type>;
  auto infer_expr_bin_expr(BinaryExpression* node) -> std::unique_ptr<Type>;
  auto infer_expr_float_literal(FloatLiteralExpression* node)
      -> std::unique_ptr<Type>;

  /// Always resolves to non-literal concrete type
  /// by defaulting literals and pushing them back down.
  /// This is basically the "YOU NEED TO RESOLVE NOW" wrapper.
  auto infer_expr_top_level(ASTNode* node) -> std::unique_ptr<Type>;

  std::unique_ptr<Type> resolve_literal_pair(const BinaryExpression* bin,
                                             std::unique_ptr<Type>& left,
                                             std::unique_ptr<Type>& right);

  VariableScopeStack variable_scope_stack;
  std::unordered_map<std::string, std::unique_ptr<Type>> function_map;
  FunctionDeclaration* current_function{};
  uint32_t loop_depth;
};

#endif  // TYPE_CHECK_VISITOR_H
