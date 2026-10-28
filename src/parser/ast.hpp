#pragma once
#include <memory>
#include <string>
#include "token.hpp"

struct Expr {
  std::size_t row, column;
  Expr(std::size_t r, std::size_t c) : row(r), column(c) {}
  virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

// Literal Expressions
struct LiteralExpr : Expr {
  std::string value;
  LiteralExpr(std::string v, std::size_t r, std::size_t c)
    : Expr(r, c), value(std::move(v)) {}
};

// Identifiers and Variables
struct IdentifierExpr : Expr {
  std::string name;
  IdentifierExpr(std::string n, std::size_t r, std::size_t c)
    : Expr(r, c), name(std::move(n)) {}
};

// a statement with two thingys(operatons?) and a operator
struct BinaryExpr : Expr {
  std::string op;
  ExprPtr left, right;
  BinaryExpr(std::string o, ExprPtr l, ExprPtr r, std::size_t row, std::size_t col)
    : Expr(row, col), op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
};
