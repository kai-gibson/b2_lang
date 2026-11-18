#ifndef COMPILE_ERROR_H
#define COMPILE_ERROR_H

#include <format>
#include <magic_enum/magic_enum_format.hpp>

#include "source_location.h"

class ParseError : public std::exception {
 public:
  template <class... Args>
  ParseError(SourceLocation source_location, std::format_string<Args...> fmt,
             Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    error = std::format("Parse Error: {} - {} line {} column {}", msg,
                        source_location.filename, source_location.line,
                        source_location.column);
  }

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return error.c_str();
  }

  std::string error;
};

class TypeError : public std::exception {
 public:
  template <class... Args>
  TypeError(SourceLocation source_location, std::format_string<Args...> fmt,
            Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    error = std::format("Type Error: {} - {} line {} column {}", msg,
                        source_location.filename, source_location.line,
                        source_location.column);
  }

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return error.c_str();
  }

  std::string error;
};

class LexerError : public std::exception {
 public:
  template <class... Args>
  LexerError(SourceLocation source_location, std::format_string<Args...> fmt,
             Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    error = std::format("Lexer Error: {} - {} line {} column {}", msg,
                        source_location.filename, source_location.line,
                        source_location.column);
  }

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return error.c_str();
  }

  std::string error;
};

class CodegenError : public std::exception {
 public:
  template <class... Args>
  CodegenError(SourceLocation source_location, std::format_string<Args...> fmt,
               Args&&... args) {
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    error = std::format("Type Error: {} - {} line {} column {}", msg,
                        source_location.filename, source_location.line,
                        source_location.column);
  }

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return error.c_str();
  }

  std::string error;
};

#endif  // COMPILE_ERROR_H
