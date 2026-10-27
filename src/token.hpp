#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

enum TokenType : uint8_t {
  Operator,
  Identifier,
  Keyword,
  Literal,
  Separator,
  EndOfFile,
  String,
  Invalid
};

struct Token {
  TokenType type;
  std::string lexeme;
  std::size_t row;
  std::size_t column;
};

constexpr std::string_view to_string(TokenType type) noexcept {
  switch (type) {
    case TokenType::EndOfFile:  return "EndOfFile";
    case TokenType::String:     return "String";
    case TokenType::Keyword:    return "Keyword";
    case TokenType::Identifier: return "Identifier";
    case TokenType::Literal:    return "Literal";
    case TokenType::Operator:   return "Operator";
    case TokenType::Separator:  return "Separator";
    case TokenType::Invalid:    return "Invalid";
  }
  return "Unknown";
}
