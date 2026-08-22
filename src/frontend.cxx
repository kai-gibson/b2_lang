#include "frontend.h"

#include <iostream>

// #include "codegen/code_generation_visitor.h"
#include "file_io.h"
#include "lexer.h"
#include "parser.h"
#include "print_visitor.h"
#include "type_check_visitor.h"

void compile(const std::string& filename, bool output_llvm,
             bool output_tokens) {
  auto file_data = read_entire_file(filename);
  if (!file_data.has_value()) {
    std::cerr << "Error reading file: " << file_data.error().what() << '\n';
  }

  Lexer lexer(FileContents{.name = filename, .data = *file_data});
  auto tokens = lexer.tokenise();
  if (output_tokens) std::cout << tokens;

  Parser parser(tokens);

  PrintVisitor v;
  auto top = parser.parse_top_level();
  // top->accept(v);
  // std::cout << v.printer.to_string() << '\n';

  TypeCheckVisitor type_checker;

  type_checker.visit_statement_node(top.get());

  top->accept(v);
  std::cout << v.printer.to_string() << '\n';

  // CodegenVisitor codegen;
  // top->accept(codegen);

  // if (output_llvm) codegen.output_llvm(filename);

  // codegen.compile();
}
