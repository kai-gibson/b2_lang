#include "operation.h"

auto token_to_binary_op(TokenType token) -> std::optional<BinaryOperation> {
  switch (token) {
    case TokenType::Plus:
      return BinaryOperation::Add;
    case TokenType::Minus:
      return BinaryOperation::Subtract;
    case TokenType::Asterisk:
      return BinaryOperation::Multiply;
    case TokenType::ForwardSlash:
      return BinaryOperation::Divide;
    case TokenType::Equals:
      return BinaryOperation::Equal;
    case TokenType::NotEquals:
      return BinaryOperation::NotEqual;
    case TokenType::GreaterThan:
      return BinaryOperation::Greater;
    case TokenType::LessThan:
      return BinaryOperation::Less;
    case TokenType::GreaterThanEquals:
      return BinaryOperation::GreaterEqual;
    case TokenType::LessThanEquals:
      return BinaryOperation::LessEqual;
    case TokenType::And:
      return BinaryOperation::And;
    case TokenType::Or:
      return BinaryOperation::Or;
    default:
      return std::nullopt;
  }
}

auto token_to_unary_op(TokenType op) -> std::optional<UnaryOperation> {
  switch (op) {
    case TokenType::Not:
      return UnaryOperation::Not;
    case TokenType::Minus:
      return UnaryOperation::Negative;
    default:
      return std::nullopt;
  }
}
