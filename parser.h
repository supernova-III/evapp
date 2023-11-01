#pragma once
#include "tokenizer.h"
#include <initializer_list>
#include <stdexcept>

template <typename T>
concept TokenType = std::is_same<T, Token::Type>::value;
// - Building AST
// - Syntax analysis with meaningful messages
// - Printing AST
// - Each AST node is mapped to the source code location
class Parser {
  TokenIterator token_iterator_;

  Token eatToken(Token::Type type);

 public:
  Parser(const char *input) : token_iterator_(input) {}
};
