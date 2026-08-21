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
