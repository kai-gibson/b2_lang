#include "lexer.h"

#include <cctype>
#include <iostream>

#include "compile_error.h"

// free pure functions
auto is_identifier_char(char chr) -> bool {
  return std::isalnum(chr) || chr == '_' || chr == '-';
}

auto is_symbol_char(char chr) -> bool {
  switch (chr) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '(':
    case ')':
    case '=':
    case ':':
    case '<':
    case '>':
    case '{':
    case '}':
    case '!':
      return true;
    default:
      return false;
  }
}

auto Lexer::current() -> char {
  return index < data.size() ? data[index] : '\0';
}

auto Lexer::peek() -> char {
  return index + 1 < data.size() ? data[index + 1] : '\0';
}

auto Lexer::parse_number() -> Token {
  size_t start = index;

  auto token_type = TokenType::IntLiteral;

  while (std::isdigit(current())) advance();

  if (current() == '.' && std::isdigit(peek())) {
    token_type = TokenType::FloatLiteral;
    advance();
    while (std::isdigit(current())) advance();
  }

  return {token_type, data.substr(start, index - start), token_location};
}

auto Lexer::parse_string_literal() -> Token {
  consume('"', "Expected '\"' to begin string literal");

  auto location = token_location;
  size_t start = index;

  while (current() != '"') {
    if (current() == '\0') {
      throw LexerError(token_location, "Unterminated string literal");
    }
    advance();
  }

  advance();
  auto string = data.substr(start, index - start);

  return {TokenType::StringLiteral, string, location};
}

auto Lexer::parse_identifier() -> Token {
  auto location = token_location;
  size_t start = index;
  while (is_identifier_char(current())) advance();
  auto word = data.substr(start, index - start);

  if (auto found = KEYWORDS.find(word); found) {
    return {*found, word, location};
  }

  return {TokenType::Identifier, word, location};
}

auto Lexer::next_token() -> Token {
  while (index < data.size() && std::isspace(current())) advance();

  if (index >= data.size() || current() == '\0') {
    return {TokenType::EndOfFile, "", token_location};
  }

  token_location = current_location;

  if (current() == '"') return parse_string_literal();
  if (is_symbol_char(current())) return parse_symbol();
  if (std::isdigit(current())) return parse_number();
  if (is_identifier_char(current())) return parse_identifier();

  current_location = token_location;
  advance();
  return {TokenType::EndOfFile, "", token_location};
}

auto Lexer::parse_symbol() -> Token {
  if (current() == '(') {
    advance();
    return {TokenType::LParen, "(", token_location};
  }
  if (current() == ')') {
    advance();
    return {TokenType::RParen, ")", token_location};
  }

  size_t start = index;
  while (is_symbol_char(current())) advance();
  auto word = data.substr(start, index - start);

  if (auto found = SYMBOLS.find(word); found.has_value()) {
    return {*found, word, token_location};
  }

  throw LexerError(token_location, "Error: Unknown token: {}", word);
}

auto Lexer::tokenise() -> Tokens {
  Tokens tokens;

  Token token = next_token();

  while (token.type != TokenType::EndOfFile) {
    tokens.push_back(token);
    token = next_token();
  }

  tokens.push_back(token);
  return tokens;
}

void Lexer::advance() {
  if (current() == '\n') {
    current_location.column = 1;
    current_location.line += 1;
  } else {
    current_location.column += 1;
  }

  index += 1;
}

void Lexer::consume(char expected, const std::string_view error_message) {
  if (current() != expected) {
    throw LexerError(token_location, "{}. Expected token: '{}', got: '{}'",
                     error_message, expected, current());
  }

  advance();
}
