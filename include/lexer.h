// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <cctype>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "source_location.h"
#include "token_type.h"

auto is_identifier_char(char chr) -> bool;
auto is_symbol_char(char chr) -> bool;

constexpr auto is_bool_operator(TokenType t) -> bool {
  constexpr auto bool_operators = to_static_set<TokenType>({
      TokenType::Equals,
      TokenType::GreaterThan,
      TokenType::GreaterThanEquals,
      TokenType::LessThan,
      TokenType::LessThanEquals,
  });

  return bool_operators.contains(t);
}

struct Token {
  TokenType type;
  std::string value;

  SourceLocation source_location;

  Token(TokenType type, std::string value, SourceLocation source_location)
      : type(type), value(std::move(value)), source_location(source_location) {}
};

inline auto operator<<(std::ostream& os, const Token& token) -> std::ostream& {
  return os << "Token{ .type=" << token_type_to_str(token.type) << " .value=\""
            << token.value << "\"}";
}

using Tokens = std::vector<Token>;

inline auto operator<<(std::ostream& os, const Tokens& tokens)
    -> std::ostream& {
  os << "[";
  for (size_t i = 0; i < tokens.size(); i++) {
    os << '\t' << tokens.at(i);
    if (i != tokens.size() - 1) os << ",\n";
  }
  return os << "]";
}

struct FileContents {
  const std::string& name;
  const std::string& data;
};

class Lexer {
 public:
  Lexer(FileContents file_contents) : data(file_contents.data) {
    current_location.filename = file_contents.name;
  }

  auto tokenise() -> Tokens;

 protected:
  SourceLocation token_location;
  SourceLocation current_location;
  const std::string& data;
  size_t index{};

  void advance();
  auto current() -> char;
  auto peek() -> char;

  auto parse_number() -> Token;
  auto parse_string_literal() -> Token;
  auto parse_identifier() -> Token;
  auto parse_symbol() -> Token;
  auto next_token() -> Token;
  void consume(char expected, const std::string_view error_message);
};

#endif  // LEXER_H
