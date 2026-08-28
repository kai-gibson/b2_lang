#include <gtest/gtest.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/PatternMatch.h>
#include <llvm/Support/TargetSelect.h>

#include "codegen/code_generation_visitor.h"
#include "lexer.h"
#include "parser.h"
#include "type_check_visitor.h"

auto codegen(const std::string& program) -> CodegenVisitor {
  auto tokens =
      Lexer(FileContents{.name = "test.b", .data = program}).tokenise();
  auto root = Parser(tokens).parse_top_level();

  auto type_checker = TypeCheckVisitor();
  type_checker.visit_statement_node(root.get());

  CodegenVisitor codegen;
  root->accept(codegen);
  return codegen;
};

auto run(const std::string& program, int64_t& out_result) -> void {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto gen = codegen(program);

  std::string error;
  llvm::EngineBuilder builder(std::move(gen.llvm_module));
  builder.setErrorStr(&error);

  llvm::ExecutionEngine* engine = builder.create();
  ASSERT_NE(engine, nullptr) << "EngineBuilder error: " << error;

  auto* func = engine->FindFunctionNamed("main");
  auto result = engine->runFunction(func, {});

  out_result = result.IntVal.getSExtValue();
}

#define RUN(OUTPUT_NAME, PROGRAM) \
  int64_t OUTPUT_NAME = 0;        \
  run(PROGRAM, OUTPUT_NAME);

/// captures stdout
#define RUN_OUTPUT(OUTPUT_NAME, STDOUT_NAME, PROGRAM) \
  ::testing::internal::CaptureStdout();               \
  int64_t OUTPUT_NAME = 0;                            \
  run(PROGRAM, OUTPUT_NAME);                          \
  std::string STDOUT_NAME = ::testing::internal::GetCapturedStdout();

TEST(CodegenTest, GeneratesSimpleFunction) {
  auto program = R"(
    func main()
      return 1
    end
  )";

  auto gen = codegen(program);
  auto func = gen.llvm_module->getFunction("main");
  ASSERT_TRUE(func->getReturnType()->isIntegerTy(32));
}

TEST(CodegenTest, GeneratesVariableDeclaration) {
  auto program = R"(
    func main()
      x = 10

      show x
      return x
    end
  )";

  RUN(output, program);
  ASSERT_EQ(output, 10);
}

TEST(CodegenTest, GeneratesVariableAssignmentSwap) {
  auto program = R"(
    func main()
      x = 123
      y = 456
      set x = y

      return x
    end
  )";

  RUN(output, program);
  ASSERT_EQ(output, 456);
}

TEST(CodegenTest, GeneratesIfStatement) {
  auto program = R"(
    func main()
      if 2 == 2
        return 8
      end

      return 1
    end
  )";

  RUN(output, program);
  ASSERT_EQ(output, 8);
}

TEST(CodegenTest, GeneratesElseIfStatement) {
  auto program = R"(
    x = 5
  
    if x > 5
      show 1 
    elseif x < 5
      show 2 
    else
      show 3 
    end
  )";

  RUN(output, program);
  ASSERT_EQ(output, 3);
}
