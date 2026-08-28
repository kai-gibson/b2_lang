#ifndef PARSER_H
#define PARSER_H

#include <algorithm>
#include <array>
#include <string_view>

#include "ast.h"
#include "lexer.h"
#include "utils/container.h"

constexpr auto is_builtin_type(std::string_view target) -> bool {
  constexpr std::array builtin_types = {
      "Int8",   "Int16",  "Int32",   "Int64",   "UInt8",  "UInt16",
      "UInt32", "UInt64", "Float32", "Float64", "String", "Bool"};

  return std::ranges::any_of(
      builtin_types,
      [target](std::string_view item) -> bool { return item == target; });
}

auto precedence(TokenType t) -> int32_t;

const std::vector<TokenType> IF_TERMINATORS = {TokenType::ElseIf,
                                               TokenType::Else, TokenType::End};

bool contains(const std::vector<TokenType>& terminators, TokenType token);

class Parser {
 public:
  Parser(const Tokens& tokens) : tokens(tokens) {}
  auto parse_top_level() -> std::unique_ptr<ASTNode>;

 private:
  void advance();
  auto peek() -> const Token&;
  auto consume(TokenType expected) -> const Token&;

  auto parse_primary_expression() -> std::unique_ptr<ASTNode>;
  auto parse_paren_expression() -> std::unique_ptr<ASTNode>;
  auto parse_float_expression() -> std::unique_ptr<ASTNode>;
  auto parse_identifier_expression() -> std::unique_ptr<ASTNode>;
  auto parse_binary_expression(std::unique_ptr<ASTNode> lhs,
                               int32_t min_precedence)
      -> std::unique_ptr<ASTNode>;

  auto parse_expression(int32_t min_precedence) -> std::unique_ptr<ASTNode>;
  auto parse_statement() -> std::unique_ptr<ASTNode>;
  auto parse_variable_declaration() -> std::unique_ptr<ASTNode>;
  auto parse_variable_assignment() -> std::unique_ptr<ASTNode>;
  auto parse_show_statement() -> std::unique_ptr<ASTNode>;
  auto parse_function_declaration() -> std::unique_ptr<ASTNode>;
  auto parse_return_statement() -> std::unique_ptr<ASTNode>;
  auto parse_type_expression() -> std::unique_ptr<ASTNode>;
  auto parse_int_expression() -> std::unique_ptr<ASTNode>;
  auto parse_if_statement() -> std::unique_ptr<ASTNode>;
  auto parse_if() -> std::unique_ptr<ASTNode>;
  auto parse_block_statement(const std::vector<TokenType>& terminators)
      -> std::unique_ptr<ASTNode>;

  auto parse_if_body(std::vector<std::unique_ptr<ASTNode>>& body) -> void;

  // vars
  const Tokens& tokens;
  std::size_t index = 0;
};

#endif  // PARSER_H
