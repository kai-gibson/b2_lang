#include <gtest/gtest.h>

#include "ast.h"
#include "node.h"
#include "type_check_visitor.h"

TEST(TypeCheckTest, InfersInt32FromLiteralDecl) {
  CHECK_EXPR(root, "x = 12");

  auto stmt = cast<VariableDeclarationStatement>(root.get());

  ASSERT_TRUE(stmt->declaration_type.has_value());
  ASSERT_EQ(stmt->declaration_type->type_id, TypeId::Int32);
}

TEST(TypeCheckTest, InfersInt32FromLiteralRet) {
  CHECK_EXPR(root, "return 12");

  auto stmt = cast<ReturnStatement>(root.get());

  ASSERT_TRUE(stmt->expr_type.has_value());
  ASSERT_EQ(stmt->expr_type->type_id, TypeId::Int32);
}

TEST(TypeCheckTest, InfersInt32FromLiteralShow) {
  CHECK_EXPR(root, "show 12");

  auto stmt = cast<ShowStatement>(root.get());

  ASSERT_TRUE(stmt->expr_type.has_value());
  ASSERT_EQ(stmt->expr_type->type_id, TypeId::Int32);
}

TEST(TypeCheckTest, InfersFloat64FromLiteralDecl) {
  CHECK_EXPR(root, "x = 12.123");

  auto stmt = cast<VariableDeclarationStatement>(root.get());

  ASSERT_TRUE(stmt->declaration_type.has_value());
  ASSERT_EQ(stmt->declaration_type->type_id, TypeId::Float64);
}

TEST(TypeCheckTest, InfersFloat64FromLiteralRet) {
  CHECK_EXPR(root, "return 12.1234");

  auto stmt = cast<ReturnStatement>(root.get());

  ASSERT_TRUE(stmt->expr_type.has_value());
  ASSERT_EQ(stmt->expr_type->type_id, TypeId::Float64);
}

TEST(TypeCheckTest, InfersFloat64FromLiteralShow) {
  CHECK_EXPR(root, "show 12.1234");

  auto stmt = cast<ShowStatement>(root.get());

  ASSERT_TRUE(stmt->expr_type.has_value());
  ASSERT_EQ(stmt->expr_type->type_id, TypeId::Float64);
}

TEST(TypeCheckTest, InfersInt64FromLiteralBinaryExpr) {
  CHECK_STMTS(stmts, "x: Int64 = 20\ny = x + 23");

  ASSERT_EQ(stmts.size(), 2);

  auto decl = cast<VariableDeclarationStatement>(stmts.at(0).get());
  ASSERT_TRUE(decl->declaration_type.has_value());
  ASSERT_EQ(decl->declaration_type->type_id, TypeId::Int64);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());
  ASSERT_TRUE(y_decl->declaration_type.has_value());
  ASSERT_EQ(y_decl->declaration_type->type_id, TypeId::Int64);
}

void test_bool_bin_expr(const std::string& code) {
  CHECK_EXPR(stmt, code);

  auto decl = cast<VariableDeclarationStatement>(stmt.get());
  ASSERT_TRUE(decl->declaration_type.has_value());

  ASSERT_EQ(decl->declaration_type->type_id, TypeId::Bool);

  auto bin = cast<BinaryExpression>(decl->value.get());

  ASSERT_TRUE(bin->operand_type.has_value());
  EXPECT_EQ(bin->operand_type->type_id, TypeId::Int32);

  ASSERT_TRUE(bin->resolved_type.has_value());
  EXPECT_EQ(bin->resolved_type->type_id, TypeId::Bool);

  auto left = cast<IntLiteralExpression>(bin->lhs.get());
  ASSERT_TRUE(left->resolved_type.has_value());
  EXPECT_EQ(left->resolved_type->type_id, TypeId::Int32);

  auto right = cast<IntLiteralExpression>(bin->rhs.get());
  ASSERT_TRUE(right->resolved_type.has_value());
  EXPECT_EQ(right->resolved_type->type_id, TypeId::Int32);
}

TEST(TypeCheckTest, InfersBoolFromLiteralBinaryExpr) {
  test_bool_bin_expr("x = 0 == 3");
}

TEST(TypeCheckTest, ChecksBoolFromLiteralBinaryExpr) {
  test_bool_bin_expr("x: Bool = 0 == 3");
}
