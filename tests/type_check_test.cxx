#include <gtest/gtest.h>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision::literals;
namespace mp = boost::multiprecision;

#include <gmock/gmock.h>

#include "ast.h"
#include "compile_error.h"
#include "node.h"

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

struct ExplicitTypeRecord {
  std::string code;
  TypeId expected;
};

class CheckExplicitTypeTest
    : public testing::TestWithParam<ExplicitTypeRecord> {};

INSTANTIATE_TEST_SUITE_P(
    ChecksExplicitType, CheckExplicitTypeTest,
    testing::Values(
        ExplicitTypeRecord{.code = "x:Int8 = 12", .expected = TypeId::Int8},
        ExplicitTypeRecord{.code = "x:Int16 = 12", .expected = TypeId::Int16},
        ExplicitTypeRecord{.code = "x:Int32 = 12", .expected = TypeId::Int32},
        ExplicitTypeRecord{.code = "x:Int64 = 12", .expected = TypeId::Int64},
        ExplicitTypeRecord{.code = "x:UInt8 = 12", .expected = TypeId::UInt8},
        ExplicitTypeRecord{.code = "x:UInt16 = 12", .expected = TypeId::UInt16},
        ExplicitTypeRecord{.code = "x:UInt32 = 12", .expected = TypeId::UInt32},
        ExplicitTypeRecord{.code = "x:UInt64 = 12", .expected = TypeId::UInt64},
        ExplicitTypeRecord{.code = "x:Float32 = 12.123",
                           .expected = TypeId::Float32},
        ExplicitTypeRecord{.code = "x:Float64 = 12.123",
                           .expected = TypeId::Float64}),
    ([](const testing::TestParamInfo<ExplicitTypeRecord>& info) -> std::string {
      return type_id_to_str(info.param.expected);
    }));

TEST_P(CheckExplicitTypeTest, ChecksExplicitType) {
  CHECK_EXPR(root, GetParam().code);

  auto stmt = cast<VariableDeclarationStatement>(root.get());

  ASSERT_TRUE(stmt->declaration_type.has_value());
  ASSERT_EQ(stmt->declaration_type->type_id, GetParam().expected);
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

TEST(TypeCheckTest, InfersLiteralAgainstConcrete) {
  CHECK_STMTS(stmts, R"(
    x: Int64 = 10
    y = x + 3
  )");

  ASSERT_EQ(stmts.size(), 2);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());

  ASSERT_TRUE(y_decl->declaration_type.has_value());
  ASSERT_EQ(y_decl->declaration_type->type_id, TypeId::Int64);

  auto bin = cast<BinaryExpression>(y_decl->value.get());

  auto left = cast<VariableExpression>(bin->lhs.get());
  auto right = cast<IntLiteralExpression>(bin->rhs.get());

  ASSERT_TRUE(left->resolved_type.has_value());
  EXPECT_EQ(left->resolved_type->type_id, TypeId::Int64);

  ASSERT_TRUE(right->resolved_type.has_value());
  EXPECT_EQ(right->resolved_type->type_id, TypeId::Int64);
}

TEST(TypeCheckTest, InfersLiteralAgainstConcreteBoolOp) {
  CHECK_STMTS(stmts, R"(
    x: Int64 = 10
    y = x > 3
  )");

  ASSERT_EQ(stmts.size(), 2);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());

  ASSERT_TRUE(y_decl->declaration_type.has_value());
  ASSERT_EQ(y_decl->declaration_type->type_id, TypeId::Bool);

  auto bin = cast<BinaryExpression>(y_decl->value.get());

  auto left = cast<VariableExpression>(bin->lhs.get());
  auto right = cast<IntLiteralExpression>(bin->rhs.get());

  ASSERT_TRUE(left->resolved_type.has_value());
  EXPECT_EQ(left->resolved_type->type_id, TypeId::Int64);

  ASSERT_TRUE(right->resolved_type.has_value());
  EXPECT_EQ(right->resolved_type->type_id, TypeId::Int64);
}

struct IntRange {
  TypeId int_type;
  std::string value;
  bool success;
};

class CheckIntFitsTest : public testing::TestWithParam<IntRange> {};

INSTANTIATE_TEST_SUITE_P(
    CheckIntFits, CheckIntFitsTest,
    testing::Values(
        IntRange{.int_type = TypeId::Int8, .value = "127", .success = true},
        IntRange{.int_type = TypeId::Int8, .value = "200", .success = false},
        IntRange{.int_type = TypeId::Int16, .value = "32767", .success = true},
        IntRange{.int_type = TypeId::Int16, .value = "50000", .success = false},
        IntRange{
            .int_type = TypeId::Int32, .value = "2147483647", .success = true},
        IntRange{
            .int_type = TypeId::Int32, .value = "2147483999", .success = false},
        IntRange{.int_type = TypeId::Int64,
                 .value = "9223372036854775807",
                 .success = true},
        IntRange{.int_type = TypeId::Int64,
                 .value = "9223372036854775999",
                 .success = false},

        IntRange{.int_type = TypeId::UInt8, .value = "255", .success = true},
        IntRange{.int_type = TypeId::UInt8, .value = "300", .success = false},
        IntRange{.int_type = TypeId::UInt16, .value = "65535", .success = true},
        IntRange{
            .int_type = TypeId::UInt16, .value = "70000", .success = false},
        IntRange{
            .int_type = TypeId::UInt32, .value = "4294967295", .success = true},
        IntRange{.int_type = TypeId::UInt32,
                 .value = "5294967295",
                 .success = false},
        IntRange{.int_type = TypeId::UInt64,
                 .value = "18446744073709551615",
                 .success = true},
        IntRange{.int_type = TypeId::UInt64,
                 .value = "18446744073709999999",
                 .success = false}),

    ([](const testing::TestParamInfo<IntRange>& info) -> std::string {
      return type_id_to_str(info.param.int_type) + "_" + info.param.value;
    }));

TEST_P(CheckIntFitsTest, CheckSizedIntFits) {
  auto param = GetParam();
  auto stmt_code =
      std::format("x: {} = {}", type_id_to_str(param.int_type), param.value);
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_EXPR(root, stmt_code);
    if (!param.success) FAIL() << "Expected out of range to throw";
  } catch (TypeError& e) {
    if (param.success) FAIL() << "Did not expect to throw. Error: " << e.what();

    ASSERT_THAT(e.what(), HasSubstr("cannot fit"));
  }
}

TEST(TypeCheckTest, ThrowsWhenVariableOutOfScope) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      x: Int64 = 10

      if x > 5
        y = x + 2
      end

      show y
    )");
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(), HasSubstr("Usage of undefined variable \"y\""));
  }
}
