#include "parser.h"

#include "compile_error.h"

// higher number == binds tighter
auto binary_binding_power(TokenType t) -> BindingPower {
  switch (t) {
    case TokenType::Or:
      return {.left = 1, .right = 2};

    case TokenType::And:
      return {.left = 3, .right = 4};

    case TokenType::Equals:
    case TokenType::NotEquals:
      return {.left = 5, .right = 6};

    case TokenType::GreaterThan:
    case TokenType::LessThan:
    case TokenType::GreaterThanEquals:
    case TokenType::LessThanEquals:
      return {.left = 7, .right = 8};

    case TokenType::Plus:
    case TokenType::Minus:
      return {.left = 9, .right = 10};

    case TokenType::Asterisk:
    case TokenType::ForwardSlash:
      return {.left = 11, .right = 12};

    default:
      return {.left = 0, .right = 0};
  }
}

auto prefix_binding_power(TokenType t) -> BindingPower {
  switch (t) {
    case TokenType::Minus:
    case TokenType::Not:
      return {.left = 0, .right = 13};

    default:
      return {.left = 0, .right = 0};
  }
}

auto postfix_binding_power(TokenType t) -> BindingPower {
  switch (t) {
    case TokenType::DotStar:
      return {.left = 14, .right = 0};

    default:
      return {.left = 0, .right = 0};
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
    throw ParseError(peek().source_location, "Expected: {}, got: {}", expected,
                     peek().type);
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
                       "Unexpected primary expression: {}", peek().type);
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

auto Parser::parse_prefix_expression(int32_t binding_power_min)
    -> std::unique_ptr<ASTNode> {
  switch (peek().type) {
    case TokenType::Minus:
    case TokenType::Not:
      return parse_unary_expression(binding_power_min);
    case TokenType::Ampersand:
      return parse_address_of_expression(binding_power_min);
    default:
      throw ParseError(peek().source_location,
                       "Unexpected prefix expression: {}", peek().type);
  }
}

auto Parser::make_postfix_expression(std::unique_ptr<ASTNode> lhs)
    -> std::unique_ptr<ASTNode> {
  auto token = peek();
  switch (token.type) {
    case TokenType::DotStar:
      advance();
      return std::make_unique<DereferenceExpression>(token.source_location,
                                                     std::move(lhs));
    default:
      throw ParseError(peek().source_location,
                       "Unexpected postfix expression: {}", peek().type);
  }
}

auto Parser::parse_expression(int32_t binding_power_min)
    -> std::unique_ptr<ASTNode> {
  std::unique_ptr<ASTNode> lhs;

  if (PREFIX_TOKENS.contains(peek().type)) {
    auto [_, right] = prefix_binding_power(peek().type);
    lhs = parse_prefix_expression(right);
  } else {
    lhs = parse_primary_expression();
  }

  while (true) {
    auto op = peek();

    if (POSTFIX_TOKENS.contains(op.type)) {
      auto [left, _] = postfix_binding_power(op.type);
      if (left <= binding_power_min) break;

      lhs = make_postfix_expression(std::move(lhs));
      continue;
    }

    auto [left, right] = binary_binding_power(op.type);
    if (left <= binding_power_min) break;

    auto binop = token_to_binary_op(op.type);
    if (!binop) {
      throw ParseError(op.source_location, "Unknown binary operator: {}",
                       op.type);
    }

    advance();
    auto rhs = parse_expression(right);
    lhs = std::make_unique<BinaryExpression>(
        *binop, std::move(lhs), std::move(rhs), op.source_location);
  }

  return lhs;
}

auto Parser::parse_type_expression() -> std::unique_ptr<ASTNode> {
  auto token = peek();
  switch (token.type) {
    case TokenType::Asterisk:
      return parse_pointer_type_expression();
    case TokenType::Identifier:
      return parse_identifier_type_expression();
    default:
      throw ParseError(token.source_location, "Invalid type expression {}",
                       token.value);
  }
}

auto Parser::parse_identifier_type_expression() -> std::unique_ptr<ASTNode> {
  auto [_, name, source_location] = consume(TokenType::Identifier);

  if (!is_builtin_type(name)) {
    throw ParseError(source_location, "Unknown type: {}", name);
  }

  return std::make_unique<TypeExpression>(name, source_location);
}

auto Parser::parse_pointer_type_expression() -> std::unique_ptr<ASTNode> {
  auto asterisk_token = consume(TokenType::Asterisk);
  auto pointer_type =
      std::make_unique<PointerTypeExpression>(asterisk_token.source_location);

  pointer_type->base_type = parse_type_expression();
  return std::move(pointer_type);
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
  auto place_expr = parse_expression(0);

  consume(TokenType::Assignment);

  auto value_expr = parse_expression(0);
  return std::make_unique<VariableAssignmentStatement>(
      std::move(place_expr), std::move(value_expr), set.source_location);
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
    case TokenType::Loop:
      return parse_loop();
    case TokenType::Break:
      return parse_break_statement();
    case TokenType::Cycle:
      return parse_cycle_statement();
    default:
      throw ParseError(peek().source_location, "Unexpected statement: {}",
                       peek().type);
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

auto Parser::parse_loop() -> std::unique_ptr<ASTNode> {
  auto loc = consume(TokenType::Loop).source_location;
  auto loop = std::make_unique<LoopStatement>(loc);

  loop->body = parse_block_statement({TokenType::End});
  consume(TokenType::End);

  return std::move(loop);
}

auto Parser::parse_break_statement() -> std::unique_ptr<ASTNode> {
  auto loc = consume(TokenType::Break).source_location;
  return std::make_unique<BreakStatement>(loc);
}

auto Parser::parse_cycle_statement() -> std::unique_ptr<ASTNode> {
  auto loc = consume(TokenType::Cycle).source_location;
  return std::make_unique<CycleStatement>(loc);
}

auto Parser::parse_unary_expression(int32_t binding_power_min)
    -> std::unique_ptr<ASTNode> {
  auto token = peek();
  advance();

  auto op = token_to_unary_op(token.type);
  if (!op) {
    throw ParseError(token.source_location, "Unknown unary operator: {}",
                     token.type);
  }

  return std::make_unique<UnaryExpression>(token.source_location, *op,
                                           parse_expression(binding_power_min));
}

auto Parser::parse_address_of_expression(int32_t binding_power_min)
    -> std::unique_ptr<ASTNode> {
  auto token = peek();
  advance();

  return std::make_unique<AddressOfExpression>(
      token.source_location, parse_expression(binding_power_min));
}
