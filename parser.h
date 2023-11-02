#pragma once
#include "tokenizer.h"
#include <stdint.h>

struct ExpressionStatement;
struct StatementList {
  struct Node {
    ExpressionStatement* expression_statement = nullptr;
    Node* next = nullptr;
  };

  Node* head = nullptr;
  Node* tail = nullptr;

  StatementList& Push(ExpressionStatement* new_statement);
};

// TODO: memory allocator for expression statements based on the expression type
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
      StatementList list;
    } block;

    struct {
      ExpressionStatement* left;
      ExpressionStatement* right;
    } assignment;
  };

  ExpressionStatement* Duplicate();
};

// - Building AST
// - Syntax analysis with meaningful messages
// - Printing AST
// - Each AST node is mapped to the source code location
class Parser {
  TokenIterator token_iterator_;

  StatementList statementList(Token::Type stopper = Token::Type::End);
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

  StatementList Run();
};
