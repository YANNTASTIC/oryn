#include "parser.hpp"
#include "parser/ast.hpp"
#include "token.hpp"
#include <memory>
#include <stdexcept>
#include <string_view>

// binding power of a binary operator, or -1 if none
static int binary_power(const Token& t) {
  if (t.type != TokenType::Operator) return -1;
  if (t.lexeme == "+" || t.lexeme == "-") return 10;
  if (t.lexeme == "*" || t.lexeme =="/") return 20;
  return -1;
};

Token Parser::advance() {
  Token t = tokens[pos];
  if (t.type != TokenType::EndOfFile) pos++;
  return t;
}

bool Parser::match(std::string_view lexeme) {
  if (lexeme == peek().lexeme) return true ;
  return false;
}

void Parser::expect(std::string_view lexeme) {
  if (!match(lexeme)) {
    throw std::runtime_error ("expected _______ found _________");
  }
}

ExprPtr Parser::parsePrimary () {
  const Token& t = peek();

  if (t.type == TokenType::Literal || t.type == TokenType::String) {
    Token tok = advance();
    return std::make_unique<LiteralExpr>(tok.lexeme, tok.row, tok.column);
  }
  if (t.type == TokenType::Identifier) {
    Token tok = advance();
    return std::make_unique<IdentifierExpr>(tok.lexeme, tok.row, tok.column);
  }

  if (match("(")) {
    ExprPtr inner = parseExpression();
    expect(")");
    return inner;
  }
  throw std::runtime_error ("exprected ________ found ______");
};


ExprPtr Parser::parseExpression (int min_power) {
  ExprPtr left = parsePrimary();

  while(true) {
    int power = binary_power(peek());
    if (power < min_power) break;
    Token op = advance();
    ExprPtr right = parseExpression(power + 1);
    left = std::make_unique<BinaryExpr>(op.lexeme, std::move(left), std::move(right), op.row, op.column);
  }
  return left;
};


