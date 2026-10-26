#include "lexer.hpp"
#include "lang.hpp"
#include "lexutils.hpp"
#include "token.hpp"

std::vector<Token> Lexer::lex() {
  std::pair<bool, Token> val = std::make_pair(true, Token{});
  std::vector<Token> tokens;
  while (val.first == true) {
    val = next();
    tokens.push_back(val.second);
  }
  return tokens;
}

bool Lexer::advance(uint n) {
  // Loop advance n times
  for (int i = 0; i < n; i++) {
    if (cursor >= source.size()) return false;

    char c = source[cursor];
    cursor++;

    if (c == '\n') {
      column = 1;
      row++;
    } else {
      column++;
    }
  }
  return cursor < source.size();
}

std::pair<bool, Token> Lexer::next() {
  while (cursor < source.size() && LexUtils::is_whitespace(peek())) {
    advance();
  }

  char c = peek();
  
  std::size_t init_row = row;
  std::size_t init_column = column;

  std::string_view lexeme;

  // Check for EOF
  if (c == '\0') {
    advance();
    return std::make_pair(false, Token{ TokenType::EndOfFile, "\0", init_row, init_column });
  }

  // Check for Alpha
  if (LexUtils::is_alpha(c)) {
    lexeme = peek_lambda([](char next){
      return LexUtils::is_alnum(next);
    });

    advance(lexeme.size());

    return std::make_pair(true, Token{ Lang::is_keyword(lexeme) ? TokenType::Keyword : TokenType::Identifier, std::string(lexeme), init_row, init_column });
  }

  // Check for Hex
  if (c == '0' && peek(1) == 'x') {
    advance(2);
    lexeme = peek_lambda([](char next){
      return LexUtils::is_xdigit(next);
    });
 
    advance(lexeme.size());

    return std::make_pair(true, Token{ TokenType::Literal, std::string("0x").append(lexeme), init_row, init_column});
  }

  // Check for Num
  if (LexUtils::is_digit(c)) {
    size_t offset = 0;

    // 1. Consume primary sequence of integer digits
    while (cursor + offset < source.size() && LexUtils::is_digit(source[cursor + offset])) {
      offset++;
    }

    // 2. Check for floating-point decimals (e.g., 1.0)
    if (cursor + offset + 1 < source.size() && 
        source[cursor + offset] == '.' && 
        LexUtils::is_digit(source[cursor + offset + 1])) {
        
      offset += 2; 
      while (cursor + offset < source.size() && LexUtils::is_digit(source[cursor + offset])) {
        offset++;
      }
    }

    // 3. Scan Dynamic Type Suffix (e.g., 10int!32, 5uint!8, 3.14float!64)
    size_t suffix_offset = offset;
    std::string_view remaining = source.substr(cursor + suffix_offset);

    // Look for any of the valid text prefixes safely
    bool has_int_prefix   = (remaining.rfind("int!", 0) == 0);
    bool has_uint_prefix  = (remaining.rfind("uint!", 0) == 0);
    bool has_float_prefix = (remaining.rfind("float!", 0) == 0);

    if (has_int_prefix || has_uint_prefix || has_float_prefix) {
      // Advance suffix pointer past the text prefix:
      // 'int!' = 4, 'uint!' = 5, 'float!' = 6 characters
      if (has_int_prefix)        suffix_offset += 4;
      else if (has_uint_prefix)  suffix_offset += 5;
      else if (has_float_prefix) suffix_offset += 6;

      // Ensure there is at least one digit following the '!'
      if (cursor + suffix_offset < source.size() && 
          LexUtils::is_digit(source[cursor + suffix_offset])) {
          
        // Consume all subsequent digits representing the bit size
        while (cursor + suffix_offset < source.size() && 
               LexUtils::is_digit(source[cursor + suffix_offset])) {
          suffix_offset++;
        }
        
        // If the pattern successfully matched, commit the entire suffix length
        offset = suffix_offset;
      }
    }

    // Extract the complete token view and advance the lexer state
    lexeme = source.substr(cursor, offset);
    advance(lexeme.size());

    return {true, Token{ TokenType::Literal, std::string(lexeme), init_row, init_column }};
  }
  
  // Check for Punc
  if (LexUtils::is_punc(c)) {
    char next = peek(1);
    if ((c == '=' && next == '=') || 
        (c == '!' && next == '=') || 
        (c == '=' && next == '>') || 
        (c == '-' && next == '>') || 
        (c == '<' && next == '>') ||
        (c == '>' && next == '=') ||
        (c == '<' && next == '=')) {
      std::string lexeme = {c, next};
      advance(2);
      return {true, Token{TokenType::Operator, lexeme, init_row, init_column}};
    }
    
    std::string strlexeme(1, c);
    advance(1);
    TokenType type;
    if (c == ';' || c == ',' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == '<' || c == '>') {
      type = TokenType::Separator;
    } else {
      type = TokenType::Operator; // Captures @, ~, &, +, -, *, /, etc.
    }
    return {true, Token{type, strlexeme, init_row, init_column}};
  }
  std::string invalid_lexeme(1, c);
  advance(1);
  return std::make_pair(true, Token{ TokenType::Invalid, invalid_lexeme, init_row, init_column });
}

char Lexer::peek(size_t offset) const noexcept {
  if (cursor + offset < source.size()) return source[cursor + offset];
  return '\0'; // Return null char if out of bounds
}

template <typename Lambda>
std::string_view Lexer::peek_lambda(Lambda&& p) const noexcept {
  size_t offset = 0;
  
  // Loop ahead from the current cursor as long as characters match
  while (cursor + offset < source.size() && p(source[cursor + offset])) {
    offset++;
  }
  
  // Return a view of what matched, starting from our current real cursor
  return source.substr(cursor, offset);
}

