#include <CLI/CLI.hpp>

#include "frontend.h"

auto main(int argc, char** argv) -> int {
  try {
    CLI::App app{"B2 compiler"};
    argv = app.ensure_utf8(argv);

    std::string filename;
    app.add_option("<filename>", filename, "Input file")->required();

    bool output_llvm{};
    app.add_flag("--output-llvm", output_llvm, "Output LLVM IR");

    bool output_tokens{};
    app.add_flag("--output-tokens", output_tokens, "Output Tokens");

    bool output_ast{};
    app.add_flag("--output-ast", output_ast, "Output AST");

    CLI11_PARSE(app, argc, argv);

    compile(filename, output_llvm, output_tokens, output_ast);
  } catch (const std::exception& e) {
    std::cout << e.what();
  }
}
