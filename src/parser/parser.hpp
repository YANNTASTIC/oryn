#pragma once
#include <vector>
#include <string_view>
#include "ast.hpp"
#include "token.hpp"

class Parser {
  public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}
    ExprPtr parseExpression(int min_power = 0);

  private:
    std::vector<Token> tokens;
    std::size_t pos = 0;

    const Token& peek() const { return tokens[pos]; }
    Token advance();
    bool match(std::string_view lexeme);
    void expect(std::string_view lexeme);

    ExprPtr parsePrimary();
};
