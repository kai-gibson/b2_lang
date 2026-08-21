#include "node.h"

auto parse_functions(const std::string& program, Nodes& nodes) -> void {
  auto tokens =
      Lexer(FileContents{.name = "test.b", .data = program}).tokenise();
  auto parser = Parser(tokens);

  auto root = parser.parse_top_level();

  auto result = dynamic_cast<Program*>(root.get());
  ASSERT_NE(result, nullptr);
  nodes = std::move(result->functions);
}

auto parse_statements(const std::string& body, Nodes& nodes) -> void {
  std::string program = "func main() " + body + " return 0 end";

  auto tokens =
      Lexer(FileContents{.name = "test.b", .data = program}).tokenise();
  auto parser = Parser(tokens);

  auto node = parser.parse_top_level();

  auto prog = dynamic_cast<Program*>(node.get());
  ASSERT_NE(prog, nullptr) << "Program not generated in parse_statements";

  ASSERT_EQ(prog->functions.size(), 1) << "Function not generated correctly";
  auto fn = dynamic_cast<FunctionDeclaration*>(prog->functions.at(0).get());
  ASSERT_NE(fn, nullptr) << "Function not generated in parse_statements";

  ASSERT_FALSE(fn->statements.empty()) << "No statements generated";
  auto ret = dynamic_cast<ReturnStatement*>(fn->statements.back().get());
  ASSERT_NE(ret, nullptr)
      << "Return statemnt not generated in parse_statements";

  fn->statements.pop_back();  // remove return statement

  nodes = std::move(fn->statements);
}

auto parse_simple_expression(const std::string& body, Node& node) -> void {
  Nodes nodes;
  ASSERT_NO_FATAL_FAILURE(parse_statements(body, nodes));

  ASSERT_GT(nodes.size(), 0) << "Failed to generate statements";
  ASSERT_EQ(nodes.size(), 1) << "Prefer parse_statements for > 1 statements";
  node = std::move(nodes.at(0));
}

auto check_functions(const std::string& program, Nodes& nodes) -> void {
  auto tokens =
      Lexer(FileContents{.name = "test.b", .data = program}).tokenise();
  auto parser = Parser(tokens);

  auto root = parser.parse_top_level();

  auto type_checker = TypeCheckVisitor();
  type_checker.visit_statement_node(root.get());

  auto result = dynamic_cast<Program*>(root.get());
  ASSERT_NE(result, nullptr);
  nodes = std::move(result->functions);
}

auto check_statements(const std::string& body, Nodes& nodes) -> void {
  std::string program = "func main() " + body + " return 0 end";

  auto tokens =
      Lexer(FileContents{.name = "test.b", .data = program}).tokenise();
  auto parser = Parser(tokens);

  auto node = parser.parse_top_level();

  auto type_checker = TypeCheckVisitor();
  type_checker.visit_statement_node(node.get());

  auto prog = dynamic_cast<Program*>(node.get());
  ASSERT_NE(prog, nullptr) << "Program not generated in parse_statements";

  ASSERT_EQ(prog->functions.size(), 1) << "Function not generated correctly";
  auto fn = dynamic_cast<FunctionDeclaration*>(prog->functions.at(0).get());
  ASSERT_NE(fn, nullptr) << "Function not generated in parse_statements";

  ASSERT_FALSE(fn->statements.empty()) << "No statements generated";
  auto ret = dynamic_cast<ReturnStatement*>(fn->statements.back().get());
  ASSERT_NE(ret, nullptr)
      << "Return statemnt not generated in parse_statements";

  fn->statements.pop_back();  // remove return statement

  nodes = std::move(fn->statements);
}

auto check_simple_expression(const std::string& body, Node& node) -> void {
  Nodes nodes;
  ASSERT_NO_FATAL_FAILURE(check_statements(body, nodes));

  ASSERT_GT(nodes.size(), 0) << "Failed to generate statements";
  ASSERT_EQ(nodes.size(), 1) << "Prefer parse_statements for > 1 statements";
  node = std::move(nodes.at(0));
}
