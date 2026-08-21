#include "parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ast.h"
#include "compile_error.h"
#include "node.h"

TEST(ParserTest, ParsesIntLiteralVarAssignment) {
  Node root;
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("x = 1234", root));
  auto decl = cast<VariableDeclarationStatement>(root.get());
  auto value = cast<IntLiteralExpression>(decl->value.get());

  ASSERT_EQ(value->value, 1234);
  EXPECT_EQ(decl->name, "x");
}

TEST(ParserTest, ParsesFloatLiteralVarAssignment) {
  Node root;
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("x = 35.131", root));
  auto decl = cast<VariableDeclarationStatement>(root.get());

  auto value = cast<FloatLiteralExpression>(decl->value.get());

  ASSERT_FLOAT_EQ(value->value, 35.131);
  EXPECT_EQ(decl->name, "x");
}

TEST(ParserTest, ParsesIntAdditionVarAssignment) {
  Node root;
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("x = 123 + 321", root));

  auto decl = cast<VariableDeclarationStatement>(root.get());

  auto value = cast<BinaryExpression>(decl->value.get());

  ASSERT_EQ(value->op, TokenType::Plus);

  auto left = cast<IntLiteralExpression>(value->lhs.get());
  auto right = cast<IntLiteralExpression>(value->rhs.get());

  ASSERT_EQ(left->value, 123);
  EXPECT_EQ(right->value, 321);
}

TEST(ParserTest, ParsesFloatAdditionVarAssignment) {
  Node root;
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("x = 1.23 + 2.34", root));
  auto decl = cast<VariableDeclarationStatement>(root.get());

  auto value = cast<BinaryExpression>(decl->value.get());
  ASSERT_EQ(value->op, TokenType::Plus);

  auto left = cast<FloatLiteralExpression>(value->lhs.get());
  auto right = cast<FloatLiteralExpression>(value->rhs.get());

  ASSERT_FLOAT_EQ(left->value, 1.23);
  EXPECT_FLOAT_EQ(right->value, 2.34);
}

TEST(ParserTest, ParsesShowStatement) {
  Node root;
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("show 123", root));
  auto show = cast<ShowStatement>(root.get());

  auto value = cast<IntLiteralExpression>(show->expr.get());

  ASSERT_EQ(value->value, 123);
}

TEST(ParserTest, ParsesSetStatement) {
  Nodes stmts;
  ASSERT_NO_FATAL_FAILURE(parse_statements("x = 10\nset x = 12", stmts));
  ASSERT_EQ(stmts.size(), 2);

  auto show = cast<VariableAssignmentStatement>(stmts.at(1).get());
  auto value = cast<IntLiteralExpression>(show->value.get());

  ASSERT_EQ(show->name, "x");
  EXPECT_EQ(value->value, 12);
}

TEST(ParserTest, ParsesMultiplication) {
  Node expr;
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("x = 10 * 20", expr));
  auto assign = cast<VariableDeclarationStatement>(expr.get());

  auto mult = cast<BinaryExpression>(assign->value.get());
  ASSERT_EQ(mult->op, TokenType::Asterisk);

  auto left = cast<IntLiteralExpression>(mult->lhs.get());
  auto right = cast<IntLiteralExpression>(mult->rhs.get());

  ASSERT_EQ(assign->name, "x");
  EXPECT_EQ(left->value, 10);
  EXPECT_EQ(right->value, 20);
}

TEST(ParserTest, ParsesDivision) {
  PARSE_EXPR(expr, "x = 10 / 20");

  auto assign = cast<VariableDeclarationStatement>(expr.get());

  auto div = cast<BinaryExpression>(assign->value.get());
  ASSERT_EQ(div->op, TokenType::ForwardSlash);
  auto left = cast<IntLiteralExpression>(div->lhs.get());
  auto right = cast<IntLiteralExpression>(div->rhs.get());

  ASSERT_EQ(assign->name, "x");
  EXPECT_EQ(left->value, 10);
  EXPECT_EQ(right->value, 20);
}

TEST(ParserTest, ParsesVariableIncrement) {
  Nodes stmts;
  ASSERT_NO_FATAL_FAILURE(parse_statements("x = 12\nset x = x * 2", stmts));
  ASSERT_EQ(stmts.size(), 2);

  auto declare = cast<VariableDeclarationStatement>(stmts.at(0).get());

  ASSERT_EQ(declare->name, "x");
  auto value = cast<IntLiteralExpression>(declare->value.get())->value;

  ASSERT_EQ(value, 12);

  auto assign = cast<VariableAssignmentStatement>(stmts.at(1).get());
  auto add = cast<BinaryExpression>(assign->value.get());

  auto left = cast<VariableExpression>(add->lhs.get());
  auto right = cast<IntLiteralExpression>(add->rhs.get());

  ASSERT_EQ(left->name, "x");
  ASSERT_EQ(right->value, 2);
}

TEST(ParserTest, ParsesTypedVariableDeclaration) {
  PARSE_EXPR(node, "x: Int32 = 1234");
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression("x: Int32 = 1234", node));

  auto decl = cast<VariableDeclarationStatement>(node.get());

  ASSERT_EQ(decl->name, "x");
  ASSERT_NE(decl->type_identifier, nullptr);

  auto type = cast<TypeExpression>(decl->type_identifier.get());
  ASSERT_EQ(type->name, "Int32");
}

TEST(ParserTest, ParsesParenExpression) {
  PARSE_EXPR(root, "x = 5 * (2 + 3)");

  auto decl = cast<VariableDeclarationStatement>(root.get());
  ASSERT_EQ(decl->name, "x");

  auto mult = cast<BinaryExpression>(decl->value.get());
  ASSERT_EQ(mult->op, TokenType::Asterisk);

  auto left_mult = cast<IntLiteralExpression>(mult->lhs.get());
  ASSERT_EQ(left_mult->value, 5);

  auto add = cast<BinaryExpression>(mult->rhs.get());
  ASSERT_EQ(add->op, TokenType::Plus);

  auto left_add = cast<IntLiteralExpression>(add->lhs.get());
  EXPECT_EQ(left_add->value, 2);

  auto right_add = cast<IntLiteralExpression>(add->rhs.get());
  EXPECT_EQ(right_add->value, 3);
}

TEST(ParserTest, ParsesMultAddOrder) {
  PARSE_EXPR(root, "x = 2 + 3 * 4");
  auto decl = cast<VariableDeclarationStatement>(root.get());
  ASSERT_EQ(decl->name, "x");

  auto add = cast<BinaryExpression>(decl->value.get());
  ASSERT_EQ(add->op, TokenType::Plus);

  auto left_add = cast<IntLiteralExpression>(add->lhs.get());
  ASSERT_EQ(left_add->value, 2);

  auto mult = cast<BinaryExpression>(add->rhs.get());
  ASSERT_EQ(mult->op, TokenType::Asterisk);

  auto mult_left = cast<IntLiteralExpression>(mult->lhs.get());
  ASSERT_EQ(mult_left->value, 3);

  auto mult_right = cast<IntLiteralExpression>(mult->rhs.get());
  ASSERT_EQ(mult_right->value, 4);
}

TEST(ParserTest, ParsesSimpleFunction) {
  constexpr auto program = R"(
    func one()
      return 1
    end

    func main() 
      return ret_one()
    end
  )";

  Nodes nodes;
  parse_functions(program, nodes);
  ASSERT_EQ(nodes.size(), 2);

  auto one = cast<FunctionDeclaration>(nodes.at(0).get());
  ASSERT_EQ(one->name, "one");
  ASSERT_EQ(one->statements.size(), 1);

  auto ret_literal = cast<ReturnStatement>(one->statements.at(0).get());
  auto int_val = cast<IntLiteralExpression>(ret_literal->expr.get());
  ASSERT_EQ(int_val->value, 1);

  auto main = cast<FunctionDeclaration>(nodes.at(1).get());
  ASSERT_EQ(main->name, "main");
  ASSERT_EQ(main->statements.size(), 1);

  auto ret_call = cast<ReturnStatement>(main->statements.at(0).get());
  auto func_call = cast<FunctionCallExpression>(ret_call->expr.get());
  ASSERT_EQ(func_call->name, "ret_one");
}

using ::testing::AnyOf;
using ::testing::HasSubstr;

// negative cases
TEST(ParserTest, ThrowsOnUnclosedParenthesis) {
  try {
    PARSE_EXPR(root, "x = 10 * (2 + 3");
  } catch (ParseError& err) {
    EXPECT_THAT(err.what(), HasSubstr("Expected ')'"));
  }
}

TEST(ParserTest, ThrowsOnUnknownType) {
  try {
    PARSE_EXPR(root, "x: FakeType = 123");
  } catch (ParseError& err) {
    EXPECT_THAT(err.what(), HasSubstr("Unknown type: FakeType"));
  }
}

TEST(ParserTest, ThrowsOnUnknownExpression) {
  try {
    PARSE_EXPR(root, "x = fafewafefdddf");
  } catch (ParseError& err) {
    EXPECT_THAT(err.what(), HasSubstr("Unexpected primary expression"));
  }
}

TEST(ParserTest, ThrowsOnUnknownStatement) {
  try {
    PARSE_EXPR(root, "*");
  } catch (ParseError& err) {
    EXPECT_THAT(err.what(), HasSubstr("Unexpected statement"));
  }
}
