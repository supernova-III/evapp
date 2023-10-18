#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct SourcePos {
  size_t line, col;
};

struct Token {
  enum Type {
    Invalid,
    End,
    NumericLiteral,
    StringLiteral,
    LeftBrace,
    RightBrace
  } type;

  union {
    double number;
    const char *string;
  };
  SourcePos pos;

  bool operator==(const Token &other) const noexcept {
    if (type != other.type) {
      return false;
    }

    switch (type) {
    case NumericLiteral:
      return number == other.number;
    case StringLiteral:
      return strcmp(string, other.string) == 0;
    case Invalid:
      return true;
    }

    return true;
  }
};

class TokenIterator {
  const char *input_;
  size_t input_length_;
  size_t cursor_ = 0;
  Token current_token_ = {};

  bool isInput() const noexcept { return cursor_ < input_length_; }

public:
  TokenIterator(const char *input)
      : input_(input), input_length_(strlen(input)), cursor_(),
        current_token_() {}

  operator bool() const noexcept {
    return current_token_.type != Token::Type::End;
  }
  const Token &next();
  const Token &peek() const { return current_token_; }
};
