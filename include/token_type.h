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
  LSquareBracket,
  RSquareBracket,
  DotStar,
  Ampersand,

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
  ElseIf,
  Loop,
  Break,
  In,
  Cycle,
  // EOF
  EndOfFile,
};

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
    {"elseif", TokenType::ElseIf},
    {"loop", TokenType::Loop},
    {"break", TokenType::Break},
    {"in", TokenType::In},
    {"cycle", TokenType::Cycle},
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
    {"[", TokenType::LSquareBracket},
    {"]", TokenType::RSquareBracket},
    {".*", TokenType::DotStar},
    {"&", TokenType::Ampersand},
});

constexpr auto PREFIX_TOKENS = to_static_set<TokenType>({
    TokenType::Minus,
    TokenType::Not,
    TokenType::Ampersand,
});

constexpr auto POSTFIX_TOKENS = to_static_set<TokenType>({
    TokenType::DotStar,
});

#endif  // TOKEN_TYPE_H
