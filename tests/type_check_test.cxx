#include <gtest/gtest.h>

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision::literals;
namespace mp = boost::multiprecision;

#include <gmock/gmock.h>

#include "ast.h"
#include "compile_error.h"
#include "node.h"
#include "types.h"

TEST(TypeCheckTest, InfersInt32FromLiteralDecl) {
  CHECK_EXPR(root, "x = 12");

  auto stmt = cast<VariableDeclarationStatement>(root.get());

  ASSERT_NE(stmt->declaration_type, nullptr);
  ASSERT_EQ(stmt->declaration_type->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, InfersInt32FromLiteralRet) {
  CHECK_EXPR(root, "return 12");

  auto stmt = cast<ReturnStatement>(root.get());

  ASSERT_NE(stmt->expr_type, nullptr);
  ASSERT_EQ(stmt->expr_type->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, InfersInt32FromLiteralShow) {
  CHECK_EXPR(root, "show 12");

  auto stmt = cast<ShowStatement>(root.get());

  ASSERT_NE(stmt->expr_type, nullptr);
  ASSERT_EQ(stmt->expr_type->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, InfersFloat64FromLiteralDecl) {
  CHECK_EXPR(root, "x = 12.123");

  auto stmt = cast<VariableDeclarationStatement>(root.get());

  ASSERT_NE(stmt->declaration_type, nullptr);
  ASSERT_EQ(stmt->declaration_type->kind(), TypeKind::Float64);
}

TEST(TypeCheckTest, InfersFloat64FromLiteralRet) {
  CHECK_EXPR(root, "return 12.1234");

  auto stmt = cast<ReturnStatement>(root.get());

  ASSERT_NE(stmt->expr_type, nullptr);
  ASSERT_EQ(stmt->expr_type->kind(), TypeKind::Float64);
}

TEST(TypeCheckTest, InfersFloat64FromLiteralShow) {
  CHECK_EXPR(root, "show 12.1234");

  auto stmt = cast<ShowStatement>(root.get());

  ASSERT_NE(stmt->expr_type, nullptr);
  ASSERT_EQ(stmt->expr_type->kind(), TypeKind::Float64);
}

TEST(TypeCheckTest, InfersInt64FromLiteralBinaryExpr) {
  CHECK_STMTS(stmts, "x: Int64 = 20\ny = x + 23");

  ASSERT_EQ(stmts.size(), 2);

  auto decl = cast<VariableDeclarationStatement>(stmts.at(0).get());
  ASSERT_NE(decl->declaration_type, nullptr);
  ASSERT_EQ(decl->declaration_type->kind(), TypeKind::Int64);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());
  ASSERT_NE(y_decl->declaration_type, nullptr);
  ASSERT_EQ(y_decl->declaration_type->kind(), TypeKind::Int64);
}

struct ExplicitTypeRecord {
  std::string code;
  TypeKind expected;
};

class CheckExplicitTypeTest
    : public testing::TestWithParam<ExplicitTypeRecord> {};

INSTANTIATE_TEST_SUITE_P(
    ChecksExplicitType, CheckExplicitTypeTest,
    testing::Values(
        ExplicitTypeRecord{.code = "x:Int8 = 12", .expected = TypeKind::Int8},
        ExplicitTypeRecord{.code = "x:Int16 = 12", .expected = TypeKind::Int16},
        ExplicitTypeRecord{.code = "x:Int32 = 12", .expected = TypeKind::Int32},
        ExplicitTypeRecord{.code = "x:Int64 = 12", .expected = TypeKind::Int64},
        ExplicitTypeRecord{.code = "x:UInt8 = 12", .expected = TypeKind::UInt8},
        ExplicitTypeRecord{.code = "x:UInt16 = 12",
                           .expected = TypeKind::UInt16},
        ExplicitTypeRecord{.code = "x:UInt32 = 12",
                           .expected = TypeKind::UInt32},
        ExplicitTypeRecord{.code = "x:UInt64 = 12",
                           .expected = TypeKind::UInt64},
        ExplicitTypeRecord{.code = "x:Float32 = 12.123",
                           .expected = TypeKind::Float32},
        ExplicitTypeRecord{.code = "x:Float64 = 12.123",
                           .expected = TypeKind::Float64}),
    ([](const testing::TestParamInfo<ExplicitTypeRecord>& info) -> std::string {
      return std::string(type_kind_to_str(info.param.expected));
    }));

TEST_P(CheckExplicitTypeTest, ChecksExplicitType) {
  CHECK_EXPR(root, GetParam().code);

  auto stmt = cast<VariableDeclarationStatement>(root.get());

  ASSERT_NE(stmt->declaration_type, nullptr);
  ASSERT_EQ(stmt->declaration_type->kind(), GetParam().expected);
}

void test_bool_bin_expr(const std::string& code) {
  CHECK_EXPR(stmt, code);

  auto decl = cast<VariableDeclarationStatement>(stmt.get());
  ASSERT_NE(decl->declaration_type, nullptr);

  ASSERT_EQ(decl->declaration_type->kind(), TypeKind::Bool);

  auto bin = cast<BinaryExpression>(decl->value.get());

  ASSERT_NE(bin->operand_type, nullptr);
  EXPECT_EQ(bin->operand_type->kind(), TypeKind::Int32);

  ASSERT_NE(bin->resolved_type, nullptr);
  EXPECT_EQ(bin->resolved_type->kind(), TypeKind::Bool);

  auto left = cast<IntLiteralExpression>(bin->lhs.get());
  ASSERT_NE(left->resolved_type, nullptr);
  EXPECT_EQ(left->resolved_type->kind(), TypeKind::Int32);

  auto right = cast<IntLiteralExpression>(bin->rhs.get());
  ASSERT_NE(right->resolved_type, nullptr);
  EXPECT_EQ(right->resolved_type->kind(), TypeKind::Int32);
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

  ASSERT_NE(y_decl->declaration_type, nullptr);
  ASSERT_EQ(y_decl->declaration_type->kind(), TypeKind::Int64);

  auto bin = cast<BinaryExpression>(y_decl->value.get());

  auto left = cast<VariableExpression>(bin->lhs.get());
  auto right = cast<IntLiteralExpression>(bin->rhs.get());

  ASSERT_NE(left->resolved_type, nullptr);
  EXPECT_EQ(left->resolved_type->kind(), TypeKind::Int64);

  ASSERT_NE(right->resolved_type, nullptr);
  EXPECT_EQ(right->resolved_type->kind(), TypeKind::Int64);
}

TEST(TypeCheckTest, InfersLiteralAgainstConcreteBoolOp) {
  CHECK_STMTS(stmts, R"(
    x: Int64 = 10
    y = x > 3
  )");

  ASSERT_EQ(stmts.size(), 2);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());

  ASSERT_NE(y_decl->declaration_type, nullptr);
  ASSERT_EQ(y_decl->declaration_type->kind(), TypeKind::Bool);

  auto bin = cast<BinaryExpression>(y_decl->value.get());

  auto left = cast<VariableExpression>(bin->lhs.get());
  auto right = cast<IntLiteralExpression>(bin->rhs.get());

  ASSERT_NE(left->resolved_type, nullptr);
  EXPECT_EQ(left->resolved_type->kind(), TypeKind::Int64);

  ASSERT_NE(right->resolved_type, nullptr);
  EXPECT_EQ(right->resolved_type->kind(), TypeKind::Int64);
}

struct IntRange {
  TypeKind int_type;
  std::string value;
  bool success;
};

class CheckIntFitsTest : public testing::TestWithParam<IntRange> {};

INSTANTIATE_TEST_SUITE_P(
    CheckIntFits, CheckIntFitsTest,
    testing::Values(
        IntRange{.int_type = TypeKind::Int8, .value = "127", .success = true},
        IntRange{.int_type = TypeKind::Int8, .value = "200", .success = false},
        IntRange{
            .int_type = TypeKind::Int16, .value = "32767", .success = true},
        IntRange{
            .int_type = TypeKind::Int16, .value = "50000", .success = false},
        IntRange{.int_type = TypeKind::Int32,
                 .value = "2147483647",
                 .success = true},
        IntRange{.int_type = TypeKind::Int32,
                 .value = "2147483999",
                 .success = false},
        IntRange{.int_type = TypeKind::Int64,
                 .value = "9223372036854775807",
                 .success = true},
        IntRange{.int_type = TypeKind::Int64,
                 .value = "9223372036854775999",
                 .success = false},

        IntRange{.int_type = TypeKind::UInt8, .value = "255", .success = true},
        IntRange{.int_type = TypeKind::UInt8, .value = "300", .success = false},
        IntRange{
            .int_type = TypeKind::UInt16, .value = "65535", .success = true},
        IntRange{
            .int_type = TypeKind::UInt16, .value = "70000", .success = false},
        IntRange{.int_type = TypeKind::UInt32,
                 .value = "4294967295",
                 .success = true},
        IntRange{.int_type = TypeKind::UInt32,
                 .value = "5294967295",
                 .success = false},
        IntRange{.int_type = TypeKind::UInt64,
                 .value = "18446744073709551615",
                 .success = true},
        IntRange{.int_type = TypeKind::UInt64,
                 .value = "18446744073709999999",
                 .success = false}),

    ([](const testing::TestParamInfo<IntRange>& info) -> std::string {
      return std::string(type_kind_to_str(info.param.int_type)) + "_" +
             info.param.value;
    }));

TEST_P(CheckIntFitsTest, CheckSizedIntFits) {
  auto param = GetParam();
  auto stmt_code =
      std::format("x: {} = {}", type_kind_to_str(param.int_type), param.value);
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
    ASSERT_TRUE(false);
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(), HasSubstr("Usage of undefined variable \"y\""));
  }
}

TEST(TypeCheckTest, ThrowsWhenCycleOutsideLoop) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      cycle
    )");
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(),
                HasSubstr("Cycle statement cannot appear outside a loop"));
  }
}

TEST(TypeCheckTest, ThrowsWhenBreakOutsideLoop) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      break
    )");
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(),
                HasSubstr("Break statement cannot appear outside a loop"));
  }
}

TEST(TypeCheckTest, InfersBoolFromNotUnary) {
  CHECK_EXPR(node, R"(
    y = not (1 == 2)
  )");

  auto y_decl = cast<VariableDeclarationStatement>(node.get());

  ASSERT_NE(y_decl->declaration_type, nullptr);
  ASSERT_EQ(y_decl->declaration_type->kind(), TypeKind::Bool);

  auto unary = cast<UnaryExpression>(y_decl->value.get());
  ASSERT_EQ(unary->resolved_type->kind(), TypeKind::Bool);

  auto bin = cast<BinaryExpression>(unary->operand.get());
  auto left = cast<IntLiteralExpression>(bin->lhs.get());
  auto right = cast<IntLiteralExpression>(bin->rhs.get());

  ASSERT_NE(left->resolved_type, nullptr);
  EXPECT_EQ(left->resolved_type->kind(), TypeKind::Int32);

  ASSERT_NE(right->resolved_type, nullptr);
  EXPECT_EQ(right->resolved_type->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, InfersInt32FromNegativeLiteral) {
  CHECK_EXPR(node, "y = -100");

  auto y_decl = cast<VariableDeclarationStatement>(node.get());

  ASSERT_NE(y_decl->declaration_type, nullptr);
  ASSERT_EQ(y_decl->declaration_type->kind(), TypeKind::Int32);

  auto unary = cast<UnaryExpression>(y_decl->value.get());
  ASSERT_EQ(unary->resolved_type->kind(), TypeKind::Int32);

  auto operand = cast<IntLiteralExpression>(unary->operand.get());
  ASSERT_NE(operand->resolved_type, nullptr);
  EXPECT_EQ(operand->resolved_type->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, InfersFloat64FromNegativeLiteral) {
  CHECK_EXPR(node, "y = -100.0");

  auto y_decl = cast<VariableDeclarationStatement>(node.get());

  ASSERT_NE(y_decl->declaration_type, nullptr);
  ASSERT_EQ(y_decl->declaration_type->kind(), TypeKind::Float64);

  auto unary = cast<UnaryExpression>(y_decl->value.get());
  ASSERT_EQ(unary->resolved_type->kind(), TypeKind::Float64);

  auto operand = cast<FloatLiteralExpression>(unary->operand.get());
  ASSERT_NE(operand->resolved_type, nullptr);
  EXPECT_EQ(operand->resolved_type->kind(), TypeKind::Float64);
}

TEST(TypeCheckTest, ChecksPointerFromAddressUnary) {
  CHECK_STMTS(stmts, R"(
    x = 10
    ptr: *Int32 = &x
  )");

  ASSERT_EQ(stmts.size(), 2);
  auto x_decl = cast<VariableDeclarationStatement>(stmts.at(0).get());
  ASSERT_NE(x_decl->declaration_type, nullptr);
  ASSERT_EQ(x_decl->declaration_type->kind(), TypeKind::Int32);

  auto ptr_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());
  ASSERT_NE(ptr_decl->declaration_type, nullptr);
  ASSERT_EQ(ptr_decl->declaration_type->kind(), TypeKind::Pointer);

  auto ptr_decl_type = cast<PointerType>(ptr_decl->declaration_type.get());
  ASSERT_EQ(ptr_decl_type->base->kind(), TypeKind::Int32);

  auto addr_of = cast<AddressOfExpression>(ptr_decl->value.get());
  ASSERT_EQ(addr_of->resolved_type->kind(), TypeKind::Pointer);

  auto addr_of_type = cast<PointerType>(addr_of->resolved_type.get());
  ASSERT_EQ(addr_of_type->base->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, InfersPointerFromAddressUnary) {
  CHECK_STMTS(stmts, R"(
    x = 10
    ptr = &x
  )");

  ASSERT_EQ(stmts.size(), 2);
  auto x_decl = cast<VariableDeclarationStatement>(stmts.at(0).get());
  ASSERT_NE(x_decl->declaration_type, nullptr);
  ASSERT_EQ(x_decl->declaration_type->kind(), TypeKind::Int32);

  auto ptr_decl = cast<VariableDeclarationStatement>(stmts.at(1).get());
  ASSERT_NE(ptr_decl->declaration_type, nullptr);
  ASSERT_EQ(ptr_decl->declaration_type->kind(), TypeKind::Pointer);

  auto ptr_decl_type = cast<PointerType>(ptr_decl->declaration_type.get());
  ASSERT_EQ(ptr_decl_type->base->kind(), TypeKind::Int32);

  auto addr_of = cast<AddressOfExpression>(ptr_decl->value.get());
  ASSERT_EQ(addr_of->resolved_type->kind(), TypeKind::Pointer);

  auto addr_of_type = cast<PointerType>(addr_of->resolved_type.get());
  ASSERT_EQ(addr_of_type->base->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, ThrowsOnNonPointerFromAddressUnary) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      x = 10
      ptr: Int32 = &x
    )");
    ASSERT_TRUE(false);
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(), HasSubstr("Address of operator '&' returns a pointer "
                                    "where Int32 was expected"));
  }
}

TEST(TypeCheckTest, ThrowsOnAddressOfValueExpr) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      ptr: *Int32 = &10
    )");
    ASSERT_TRUE(false);
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(), HasSubstr("Cannot use address of operator '&' on "
                                    "value expression: IntLiteralExpression"));
  }
}

TEST(TypeCheckTest, ThrowsOnAddressOfInferValueExpr) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      ptr = &10
    )");
    ASSERT_TRUE(false);
  } catch (TypeError& e) {
    ASSERT_THAT(e.what(), HasSubstr("Cannot use address of operator '&' on "
                                    "value expression: IntLiteralExpression"));
  }
}

TEST(TypeCheckTest, ThrowsOnAddressOfWrongPlaceExprType) {
  using ::testing::HasSubstr;
  using ::testing::Not;

  try {
    CHECK_STMTS(stmts, R"(
      x: Int16 = 1
      ptr: *Int32 = &x
    )");
    ASSERT_TRUE(false);
  } catch (TypeError& e) {
    ASSERT_THAT(
        e.what(),
        HasSubstr("Expected place expression to have type Int32. Got Int16"));
  }
}

TEST(TypeCheckTest, InfersInt32FromDeref) {
  CHECK_STMTS(stmts, R"(
    x = 10
    ptr = &x
    y = ptr.*
  )");

  ASSERT_EQ(stmts.size(), 3);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(2).get());
  auto deref = cast<DereferenceExpression>(y_decl->value.get());

  ASSERT_EQ(deref->resolved_type->kind(), TypeKind::Int32);
}

TEST(TypeCheckTest, ChecksInt32FromDeref) {
  CHECK_STMTS(stmts, R"(
    x = 10
    ptr = &x
    y: Int32 = ptr.*
  )");

  ASSERT_EQ(stmts.size(), 3);

  auto y_decl = cast<VariableDeclarationStatement>(stmts.at(2).get());
  auto deref = cast<DereferenceExpression>(y_decl->value.get());

  ASSERT_EQ(deref->resolved_type->kind(), TypeKind::Int32);
}
