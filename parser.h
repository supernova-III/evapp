#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unordered_set>
#include <string_view>

struct Token {
  enum Type : uint8_t {
    Type_Invalid,
    Type_End,
    Type_NumericLiteral,
    Type_StringLiteral,
    Type_LeftBrace,
    Type_RightBrace,
    Type_Plus,
    Type_Minus,
    Type_Multiply,
    Type_Divide,
    Type_Assign,
    Type_Identifier
  } type;

  union {
    double number;
    const char* string;
  };

  bool operator==(const Token& other) const noexcept {
    if (type != other.type) {
      return false;
    }

    switch (type) {
      case Token::Type_NumericLiteral:
        return number == other.number;
      case Token::Type_StringLiteral:
        return strcmp(string, other.string) == 0;
      default:
        break;
    }

    return true;
  }
};

class TokenIterator {
  const char* input_;
  size_t input_length_;
  size_t cursor_ = 0;
  Token current_token_ = {};
  std::unordered_set<std::string_view> identifiers_ = {};

  bool isInput() const noexcept { return cursor_ < input_length_; }

 public:
  TokenIterator(const char* input)
      : input_(input),
        input_length_(strlen(input)),
        cursor_(),
        current_token_() {}

  operator bool() const noexcept {
    return current_token_.type != Token::Type_End;
  }
  const Token& Next();
  const Token& Peek() const { return current_token_; }
};

struct ExpressionStatement {
  enum Type : uint8_t {
    Type_Block,
    Type_Binary,
    Type_NumericLiteral,
    Type_StringLiteral,
    Type_Assignment,
    Type_Identifier,
  } type;

  union {
    struct {
      ExpressionStatement* left;
      ExpressionStatement* right;
      Token op;
    } binary;

    struct {
      Token literal;
    } numeric_literal;

    struct {
      Token literal;
    } string_literal;

    struct {
      Token name;
    } identifier;

    struct {
      Token starter;
      ExpressionStatement* head = nullptr;
    } block;

    struct {
      ExpressionStatement* left;
      ExpressionStatement* right;
    } assignment;
  };
  ExpressionStatement* next = nullptr;

  ExpressionStatement* Duplicate();
  void DumpJsonToFile(FILE* file);
};

// - Building AST
// - Syntax analysis with meaningful messages
// - Printing AST
// - Each AST node is mapped to the source code location
class Parser {
  TokenIterator token_iterator_;

  ExpressionStatement* statementList(Token starter,
                                     Token::Type stopper = Token::Type_End);
  ExpressionStatement* assignment();
  ExpressionStatement* stringLiteral();
  ExpressionStatement* numericLiteral();
  ExpressionStatement* additiveExpression();
  ExpressionStatement* multiplicativeExpression();
  ExpressionStatement* expressionStatement();
  ExpressionStatement* primaryStatement();

  Token consumeToken(Token::Type token_type);

 public:
  Parser(const char* input) : token_iterator_(input) {}

  ExpressionStatement* Run();
};
