#include "parser.h"

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

bool contains(const std::vector<TokenType>& terminators, TokenType token) {
  for (const auto term : terminators) {
    if (term == token) return true;
  }

  return false;
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
  auto value = mp::int128_t(token.value);
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
    case TokenType::If:
      return parse_if_statement();
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

  func->statements = parse_block_statement({TokenType::End});
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

constexpr auto branches =
    to_static_set({TokenType::Else, TokenType::ElseIf, TokenType::End});

auto Parser::parse_if_body(std::vector<std::unique_ptr<ASTNode>>& body)
    -> void {
  while (!branches.contains(peek().type)) {
    body.push_back(parse_statement());
  }
}

auto Parser::parse_if_statement() -> std::unique_ptr<ASTNode> {
  SourceLocation begin;
  if (peek().type == TokenType::If) {
    begin = consume(TokenType::If).source_location;
  } else if (peek().type == TokenType::ElseIf) {
    begin = consume(TokenType::ElseIf).source_location;
  }

  auto if_stmt = std::make_unique<IfStatement>(begin);

  if_stmt->condition = parse_expression(0);

  if_stmt->if_then = parse_block_statement(IF_TERMINATORS);

  if (peek().type == TokenType::ElseIf) {
    if_stmt->if_else = parse_if_statement();
  } else if (peek().type == TokenType::Else) {
    consume(TokenType::Else);
    if_stmt->if_else = parse_block_statement({TokenType::End});
  }

  if (peek().type == TokenType::End) {
    consume(TokenType::End);
  }

  return std::move(if_stmt);
}

auto Parser::parse_block_statement(const std::vector<TokenType>& terminators)
    -> std::unique_ptr<ASTNode> {
  auto block = std::make_unique<BlockStatement>(peek().source_location);

  while (!contains(terminators, peek().type)) {
    block->statements.push_back(parse_statement());
  }

  return std::move(block);
}
