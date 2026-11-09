
#include "lexer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <ostream>

#include "compile_error.h"

inline void PrintTo(const TokenType& type, std::ostream* os) {
  *os << token_type_to_str(type);
}

TEST(IsIdentifierCharTest, WorksForAllIdentifiersTest) {
  for (char chr = '!'; chr <= '~'; chr++) {
    bool expected = std::isalnum(chr) || chr == '_' || chr == '-';
    EXPECT_EQ(is_identifier_char(chr), expected)
        << "Failed for char: '" << chr << "' (0x" << std::hex << (int)chr
        << ")";
  }
}

constexpr auto VALID_SYMBOLS = std::to_array({
    '+',
    '-',
    '*',
    '/',
    '(',
    ')',
    '=',
    ':',
    '<',
    '>',
    '{',
    '}',
    '!',
    '*',
});

TEST(IsSymbolCharTest, FailsOnInvalidSymbols) {
  for (char chr = '!'; chr <= '~'; chr++) {
    using std::ranges::contains;

    if (!contains(VALID_SYMBOLS, chr)) {
      EXPECT_EQ(is_symbol_char(chr), false)
          << "Failed for char: '" << chr << "' (0x" << std::hex << (int)chr
          << ")";
    }
  }
}

TEST(IsSymbolCharTest, WorksForAllValidSymbolsTest) {
  for (const auto chr : VALID_SYMBOLS) {
    bool expected =
        std::isprint(chr) && !std::isalnum(chr) && !std::isspace(chr);

    EXPECT_EQ(is_symbol_char(chr), expected)
        << "Failed for char: '" << chr << "' (0x" << std::hex << (int)chr
        << ")";
  }
}

// helper for lexer public api
auto lex_string(const std::string& data, const std::string& filename = "test.b")
    -> Tokens {
  auto lexer = Lexer(FileContents{.name = filename, .data = data});
  return lexer.tokenise();
}

TEST(LexerEdgeCases, HandlesEmptyInputTest) {
  auto tokens = lex_string("");
  EXPECT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens.at(0).type, TokenType::EndOfFile);
}

TEST(LexerEdgeCases, SkipsWhitespaceTest) {
  auto tokens = lex_string(" \n\n");
  EXPECT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens.at(0).type, TokenType::EndOfFile);
}

TEST(LexerEdgeCases, SkipsInvalidUnicodeTest) {
  auto tokens = lex_string("\u0444");
  EXPECT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens.at(0).type, TokenType::EndOfFile);
}

TEST(LexerEdgeCases, ThrowsLexerErrorOnUnterminatedStringLiteral) {
  ASSERT_THROW(([] -> void { auto _ = lex_string("\"Unterminated..."); }()),
               LexerError);
}

struct LexerData {
  std::string name;
  const std::string input;
  std::vector<TokenType> expected;

  friend void PrintTo(const LexerData& lexer_data, std::ostream* os) {
    *os << std::format("(\"{}\", {})", lexer_data.input,
                       token_type_to_str(lexer_data.expected.at(0)));
  }
};

class LexerParamTest : public testing::TestWithParam<LexerData> {};

INSTANTIATE_TEST_SUITE_P(
    BasicTokens, LexerParamTest,
    testing::Values(
        LexerData{"Plus", "+", {TokenType::Plus, TokenType::EndOfFile}},
        LexerData{"Minus", "-", {TokenType::Minus, TokenType::EndOfFile}},
        LexerData{"Asterisk", "*", {TokenType::Asterisk, TokenType::EndOfFile}},
        LexerData{"ForwardSlash",
                  "/",
                  {TokenType::ForwardSlash, TokenType::EndOfFile}},
        LexerData{"FloatLiteral",
                  "1.425",
                  {TokenType::FloatLiteral, TokenType::EndOfFile}},
        LexerData{"IntLiteral",
                  "3112",
                  {TokenType::IntLiteral, TokenType::EndOfFile}},
        LexerData{"StringLiteral",
                  "\"testing\"",
                  {TokenType::StringLiteral, TokenType::EndOfFile}},
        LexerData{"Identifier",
                  "somevar",
                  {TokenType::Identifier, TokenType::EndOfFile}},
        LexerData{"LParen", "(", {TokenType::LParen, TokenType::EndOfFile}},
        LexerData{"RParen", ")", {TokenType::RParen, TokenType::EndOfFile}},
        LexerData{
            "Assignment", "=", {TokenType::Assignment, TokenType::EndOfFile}},
        LexerData{"Colon", ":", {TokenType::Colon, TokenType::EndOfFile}},
        LexerData{"LessThan", "<", {TokenType::LessThan, TokenType::EndOfFile}},
        LexerData{
            "GreaterThan", ">", {TokenType::GreaterThan, TokenType::EndOfFile}},
        LexerData{"LessThanEquals",
                  "<=",
                  {TokenType::LessThanEquals, TokenType::EndOfFile}},
        LexerData{"GreaterThanEquals",
                  ">=",
                  {TokenType::GreaterThanEquals, TokenType::EndOfFile}},
        LexerData{"Equals", "==", {TokenType::Equals, TokenType::EndOfFile}},
        LexerData{
            "NotEquals", "!=", {TokenType::NotEquals, TokenType::EndOfFile}},
        LexerData{"Set", "set", {TokenType::Set, TokenType::EndOfFile}},
        LexerData{"Show", "show", {TokenType::Show, TokenType::EndOfFile}},
        LexerData{
            "Function", "func", {TokenType::Function, TokenType::EndOfFile}},
        LexerData{"End", "end", {TokenType::End, TokenType::EndOfFile}},
        LexerData{
            "Return", "return", {TokenType::Return, TokenType::EndOfFile}},
        LexerData{"And", "and", {TokenType::And, TokenType::EndOfFile}},
        LexerData{"Or", "or", {TokenType::Or, TokenType::EndOfFile}},
        LexerData{"If", "if", {TokenType::If, TokenType::EndOfFile}},
        LexerData{"Else", "else", {TokenType::Else, TokenType::EndOfFile}},
        LexerData{"Not", "not", {TokenType::Not, TokenType::EndOfFile}}),
    ([](const testing::TestParamInfo<LexerData>& info) -> std::string {
      return info.param.name;
    }));

TEST_P(LexerParamTest, ProducesExpectedTokenTypes) {
  auto tokens = lex_string(GetParam().input);

  ASSERT_EQ(tokens.size(), GetParam().expected.size())
      << "size incorrect [" << token_type_to_str(tokens[0].type) << ","
      << token_type_to_str(tokens[1].type) << "]";

  for (size_t i = 0; i < tokens.size(); i++) {
    EXPECT_EQ(tokens[i].type, GetParam().expected[i]);
  }
}
