#include "parser.h"

#include <iostream>

#include "compile_error.h"

// higher number == binds tighter
auto precedence(TokenType t) -> int32_t {
  switch (t) {
    case TokenType::Equals:
    case TokenType::NotEquals:
      return 1;
    case TokenType::GreaterThan:
    case TokenType::LessThan:
    case TokenType::GreaterThanEquals:
    case TokenType::LessThanEquals:
      return 2;
    case TokenType::Plus:
    case TokenType::Minus:
      return 3;
    case TokenType::Asterisk:
    case TokenType::ForwardSlash:
      return 4;
    default:
      return 0;
  }
}

auto Parser::consume(TokenType expected) -> const Token& {
  if (peek().type != expected) {
    throw ParseError(peek().source_location, "Expected: {}, got: {}",
                     token_type_to_str(expected),
                     token_type_to_str(peek().type));
  }

  auto& token = peek();
  advance();
  return token;
}

auto Parser::peek() -> const Token& {
  // return EOF token instead of OOB
  return (index < tokens.size()) ? tokens[index] : tokens.back();
}

void Parser::advance() { index += 1; }

auto Parser::parse_float_expression() -> std::unique_ptr<ASTNode> {
  auto token = peek();
  std::clog << "converting: " << token.value << " to double...\n";
  auto value = std::stod(token.value);
  auto result =
      std::make_unique<FloatLiteralExpression>(value, token.source_location);
  advance();

  return std::move(result);
}

auto Parser::parse_paren_expression() -> std::unique_ptr<ASTNode> {
  advance();
  auto expr = parse_expression(0);
  if (peek().type != TokenType::RParen) {
    throw ParseError(peek().source_location, "Expected ')'");
  }

  advance();
  return expr;
}

auto Parser::parse_identifier_expression() -> std::unique_ptr<ASTNode> {
  std::string name = peek().value;
  advance();

  // Function call expression
  if (peek().type == TokenType::LParen) {
    // No args implemented yet
    advance();
    consume(TokenType::RParen);
    return std::make_unique<FunctionCallExpression>(name,
                                                    peek().source_location);
  }

  return std::make_unique<VariableExpression>(name, peek().source_location);
}

auto Parser::parse_primary_expression() -> std::unique_ptr<ASTNode> {
  switch (peek().type) {
    case TokenType::FloatLiteral:
      return parse_float_expression();
    case TokenType::IntLiteral:
      return parse_int_expression();
    case TokenType::LParen:
      return parse_paren_expression();
    case TokenType::Identifier:
      return parse_identifier_expression();
    default:
      throw ParseError(peek().source_location,
                       "Unexpected primary expression: {}",
                       token_type_to_str(peek().type));
  }
}

auto Parser::parse_int_expression() -> std::unique_ptr<ASTNode> {
  auto token = peek();
  std::clog << "converting: " << token.value << " to Int32...\n";
  auto value = std::stoi(token.value);
  auto result =
      std::make_unique<IntLiteralExpression>(value, peek().source_location);
  advance();

  return std::move(result);
}

auto Parser::parse_expression(int32_t min_precedence)
    -> std::unique_ptr<ASTNode> {
  auto lhs = parse_primary_expression();

  while (precedence(peek().type) > min_precedence) {
    auto op = peek();
    advance();
    auto rhs = parse_expression(precedence(op.type));
    lhs = std::make_unique<BinaryExpression>(
        op.type, std::move(lhs), std::move(rhs), op.source_location);
  }

  return lhs;
}

auto Parser::parse_type_expression() -> std::unique_ptr<ASTNode> {
  auto [_, name, source_location] = consume(TokenType::Identifier);

  if (!is_builtin_type(name)) {
    throw ParseError(source_location, "Unknown type: {}", name);
  }

  return std::make_unique<TypeExpression>(name, source_location);
}

auto Parser::parse_variable_declaration() -> std::unique_ptr<ASTNode> {
  auto name = peek();
  advance();

  std::unique_ptr<ASTNode> type_identifier = nullptr;
  // explicitly typed variable
  if (peek().type == TokenType::Colon) {
    advance();

    type_identifier = parse_type_expression();
  }

  consume(TokenType::Assignment);
  return std::make_unique<VariableDeclarationStatement>(
      name.value, parse_expression(0), name.source_location,
      std::move(type_identifier));
};

auto Parser::parse_variable_assignment() -> std::unique_ptr<ASTNode> {
  auto set = consume(TokenType::Set);
  auto [_, name, _a] = consume(TokenType::Identifier);
  consume(TokenType::Assignment);

  return std::make_unique<VariableAssignmentStatement>(
      name, parse_expression(0), set.source_location);
}

auto Parser::parse_show_statement() -> std::unique_ptr<ASTNode> {
  auto show = consume(TokenType::Show);

  return std::make_unique<ShowStatement>(parse_expression(0),
                                         show.source_location);
}

auto Parser::parse_statement() -> std::unique_ptr<ASTNode> {
  switch (peek().type) {
    case TokenType::Identifier:
      return parse_variable_declaration();
    case TokenType::Set:
      return parse_variable_assignment();
    case TokenType::Show:
      return parse_show_statement();
    case TokenType::Return:
      return parse_return_statement();
    default:
      throw ParseError(peek().source_location, "Unexpected statement: {}",
                       token_type_to_str(peek().type));
  }
};

auto Parser::parse_function_declaration() -> std::unique_ptr<ASTNode> {
  consume(TokenType::Function);
  auto [_, name, source_location] = consume(TokenType::Identifier);

  auto func = std::make_unique<FunctionDeclaration>(name, source_location);

  consume(TokenType::LParen);
  consume(TokenType::RParen);

  while (peek().type != TokenType::End) {
    func->statements.push_back(parse_statement());
  }

  consume(TokenType::End);

  return std::move(func);
}

auto Parser::parse_top_level() -> std::unique_ptr<ASTNode> {
  auto program = std::make_unique<Program>();

  while (peek().type != TokenType::EndOfFile) {
    program->functions.push_back(parse_function_declaration());
  }

  return std::move(program);
}

auto Parser::parse_return_statement() -> std::unique_ptr<ASTNode> {
  auto ret = consume(TokenType::Return);

  return std::make_unique<ReturnStatement>(parse_expression(0),
                                           ret.source_location);
}
