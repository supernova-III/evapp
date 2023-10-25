#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct SourcePos {
  size_t line, col;

  operator bool() const noexcept { return line != 0 && col != 0; }
  bool operator==(const SourcePos &other) const noexcept {
    return line == other.line && col == other.col;
  }
  bool operator!=(const SourcePos &other) const noexcept {
    return !(*this == other);
  }
};

struct Token {
  enum struct Type {
    Invalid,
    End,
    NumericLiteral,
    StringLiteral,
    LeftBrace,
    RightBrace,
    Plus,
    Minus,
    Multiply,
    Divide
  } type;

  union {
    double number;
    const char *string;
  };
  SourcePos pos = {1, 1};

  static Token StringLiteral(const char *string) {
    return {.type = Type::StringLiteral, .string = string};
  }

  static Token NumericLiteral(double number) {
    return {.type = Type::NumericLiteral, .number = number};
  }

  bool operator==(const Token &other) const noexcept {
    if (type != other.type) {
      return false;
    }

    if (pos != other.pos) {
      return false;
    }

    switch (type) {
      case Type::NumericLiteral:
        return number == other.number;
      case Type::StringLiteral:
        return strcmp(string, other.string) == 0;
    }

    return true;
  }
};

class TokenIterator {
  const char *input_;
  size_t input_length_;
  size_t cursor_ = 0;
  Token current_token_ = {};
  SourcePos current_pos_ = {1, 1};

  bool isInput() const noexcept { return cursor_ < input_length_; }

 public:
  TokenIterator(const char *input)
      : input_(input),
        input_length_(strlen(input)),
        cursor_(),
        current_token_() {}

  operator bool() const noexcept {
    return current_token_.type != Token::Type::End;
  }
  const Token &Next();
  const Token &Peek() const { return current_token_; }
};
