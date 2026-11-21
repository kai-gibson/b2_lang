#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <cstdint>

#include "token_type.h"

enum class BinaryOperation : uint8_t {
  Add,
  Subtract,
  Multiply,
  Divide,
  Equal,
  NotEqual,
  Greater,
  Less,
  GreaterEqual,
  LessEqual,
  And,
  Or,
};

enum class UnaryOperation : uint8_t {
  Not,
  Negative,
};

auto token_to_unary_op(TokenType op) -> std::optional<UnaryOperation>;
auto token_to_binary_op(TokenType token) -> std::optional<BinaryOperation>;

#endif  // OPERATIONS_H
