#pragma once
#include "token.hpp"
#include <vector>
#include <string_view>
#include <string>

class Lexer {
  public:
    std::string_view source;

    std::vector<Token> lex();

    explicit Lexer(std::string_view src) : source(src) {}
  private:
    std::size_t cursor = 0;
    std::size_t row = 1;
    std::size_t column = 1;

    bool advance(uint n = 1);

    std::pair<bool, Token> next();

    char peek(size_t offset = 0) const noexcept;

    template <typename Lambda>
    std::string_view peek_lambda(Lambda&& p) const noexcept;
};
