#ifndef NODE_H
#define NODE_H

#include <gtest/gtest.h>

#include <format>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "type_check_visitor.h"

using Nodes = std::vector<std::unique_ptr<ASTNode>>;
using Node = std::unique_ptr<ASTNode>;

// template <class T>
// auto cast(Node& node, T&& output) -> void {
//   auto ptr = dynamic_cast<T*>(node.get());
//   ASSERT_NE(ptr, nullptr);
//   output = ptr;
// }
//
// template <class T>
// auto cast_node(Node& node) -> T* {
//   auto ptr = dynamic_cast<T*>(node.get());
//   if (!ptr)
//     throw std::runtime_error(
//         std::format("Failed to cast node to type: {}", typeid(T).name()));
//   return ptr;
// }

auto parse_functions(const std::string& program, Nodes& nodes) -> void;

auto parse_statements(const std::string& body, Nodes& nodes) -> void;

auto parse_simple_expression(const std::string& body, Node& node) -> void;

// I would never use this in prod code, but it's great here.
#define PARSE_EXPR(NAME, EXPR) \
  Node NAME;                   \
  ASSERT_NO_FATAL_FAILURE(parse_simple_expression(EXPR, NAME));

#define PARSE_STMTS(NAME, EXPR) \
  Nodes NAME;                   \
  ASSERT_NO_FATAL_FAILURE(parse_statements(EXPR, NAME));

#define PARSE_FNS(NAME, EXPR) \
  Program* NAME = nullptr;    \
  ASSERT_NO_FATAL_FAILURE(parse_functions(EXPR, NAME));

auto check_functions(const std::string& program, Nodes& nodes) -> void;
auto check_statements(const std::string& body, Nodes& nodes) -> void;
auto check_simple_expression(const std::string& body, Node& node) -> void;

// I would never use this in prod code, but it's great here.
#define CHECK_EXPR(NAME, EXPR) \
  Node NAME;                   \
  ASSERT_NO_FATAL_FAILURE(check_simple_expression(EXPR, NAME));

#define CHECK_STMTS(NAME, EXPR) \
  Nodes NAME;                   \
  ASSERT_NO_FATAL_FAILURE(check_statements(EXPR, NAME));

#define CHECK_FNS(NAME, EXPR) \
  Program* NAME = nullptr;    \
  ASSERT_NO_FATAL_FAILURE(check_functions(EXPR, NAME));
#endif  // NODE_H
