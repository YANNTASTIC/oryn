#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <file.rn>\n";
    return 1;
  }

  std::ifstream file(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "error: cannot open " << argv[1] << "\n";
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();

  Lexer lexer(source);
  std::vector<Token> tokens = lexer.lex(); 

  for (auto token : tokens) {
    std::cout << "[" << to_string(token.type) << "," << token.lexeme << "," << std::to_string(token.row) << "," << std::to_string(token.column) << "]" << std::endl;

  }
  return 0;
}
