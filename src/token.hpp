#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

enum TokenType : uint8_t {
  Operator,
  Identifier,
  Keyword,
  Literal,
  Separator,
  EndOfFile,
  Invalid
};

struct Token {
  TokenType type;
  std::string lexeme;
  std::size_t row;
  std::size_t column;
};

