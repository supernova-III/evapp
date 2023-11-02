#pragma once
#include <stddef.h>
#include <string.h>
#include <unordered_set>
#include <string_view>

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
    Divide,
    Assign,
    Identifier
  } type;

  union {
    double number;
    const char *string;
  };

  bool operator==(const Token &other) const noexcept {
    if (type != other.type) {
      return false;
    }

    switch (type) {
      case Type::NumericLiteral:
        return number == other.number;
      case Type::StringLiteral:
        return strcmp(string, other.string) == 0;
      default:
        break;
    }

    return true;
  }
};

class TokenIterator {
  const char *input_;
  size_t input_length_;
  size_t cursor_ = 0;
  Token current_token_ = {};
  std::unordered_set<std::string_view> identifiers_ = {};

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
