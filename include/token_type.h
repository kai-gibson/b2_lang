#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <string_view>

#include "utils/container.h"

enum class TokenType : int8_t {
  Plus,
  Minus,
  Asterisk,
  ForwardSlash,
  FloatLiteral,
  IntLiteral,
  StringLiteral,
  Identifier,
  LParen,
  RParen,
  Assignment,
  Colon,
  LessThan,
  GreaterThan,
  LessThanEquals,
  GreaterThanEquals,
  Equals,
  NotEquals,

  // Keywords
  Set,
  Show,
  Function,
  End,
  Return,
  And,
  Or,
  If,
  Else,
  Not,
  // EOF
  EndOfFile,
};

constexpr auto TOKEN_TYPE_STR = to_static_map<TokenType, std::string_view>({
    {TokenType::Plus, "Plus"},
    {TokenType::Minus, "Minus"},
    {TokenType::Asterisk, "Asterisk"},
    {TokenType::ForwardSlash, "ForwardSlash"},
    {TokenType::FloatLiteral, "FloatLiteral"},
    {TokenType::IntLiteral, "IntLiteral"},
    {TokenType::StringLiteral, "StringLiteral"},
    {TokenType::Identifier, "Identifier"},
    {TokenType::LParen, "LParen"},
    {TokenType::RParen, "RParen"},
    {TokenType::Assignment, "Assignment"},
    {TokenType::Colon, "Colon"},
    {TokenType::LessThan, "LessThan"},
    {TokenType::GreaterThan, "GreaterThan"},
    {TokenType::LessThanEquals, "LessThanEquals"},
    {TokenType::GreaterThanEquals, "GreaterThanEquals"},
    {TokenType::Equals, "Equals"},
    {TokenType::NotEquals, "NotEquals"},
    {TokenType::Set, "Set"},
    {TokenType::Show, "Show"},
    {TokenType::Function, "Function"},
    {TokenType::End, "End"},
    {TokenType::Return, "Return"},
    {TokenType::And, "And"},
    {TokenType::Or, "Or"},
    {TokenType::If, "If"},
    {TokenType::Else, "Else"},
    {TokenType::Not, "Not"},
    {TokenType::EndOfFile, "EndOfFile"},
});

constexpr auto KEYWORDS = to_static_map<std::string_view, TokenType>({
    {"set", TokenType::Set},
    {"show", TokenType::Show},
    {"func", TokenType::Function},
    {"end", TokenType::End},
    {"return", TokenType::Return},
    {"and", TokenType::And},
    {"or", TokenType::Or},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"not", TokenType::Not},
});

constexpr auto SYMBOLS = to_static_map<std::string_view, TokenType>({
    {"+", TokenType::Plus},
    {"-", TokenType::Minus},
    {"*", TokenType::Asterisk},
    {"/", TokenType::ForwardSlash},
    {"(", TokenType::LParen},
    {")", TokenType::RParen},
    {"=", TokenType::Assignment},
    {":", TokenType::Colon},
    {"<", TokenType::LessThan},
    {">", TokenType::GreaterThan},
    {"<=", TokenType::LessThanEquals},
    {">=", TokenType::GreaterThanEquals},
    {"==", TokenType::Equals},
    {"!=", TokenType::NotEquals},
});

constexpr auto token_type_to_str(TokenType t) -> const std::string_view {
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  return TOKEN_TYPE_STR.find(t)->get();
}

#endif  // TOKEN_TYPE_H
