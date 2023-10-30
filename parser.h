#pragma once
#include "tokenizer.h"
#include <initializer_list>
#include <stdio.h>

struct Statement;

struct StatementList {
  struct Node {
    Statement *statement;
    Node *next;
  };
  Node *head = nullptr;
  Node *tail = nullptr;

  StatementList(std::initializer_list<Statement *> statements = {});
  StatementList &AddStatement(Statement *s);
  void Print(FILE *file);
};

struct ExpressionStatement;
struct AdditiveExpression {
  enum struct Type { Primary, Complex } type;

  union {
    ExpressionStatement *primary;
    struct {
      Token additive_operator;
      AdditiveExpression *additive_expression;
      ExpressionStatement *primary;
    } complex;
  };
  void Print(FILE *file);
};

struct Literal {
  enum struct Type { String, Number } type;
  Token token;
  void Print(FILE *file);
};

struct PrimaryExpression {
  enum struct Type { Literal } type;
  union {
    Literal literal;
  };
  void Print(FILE *file);
};

struct ExpressionStatement {
  enum struct Type { PrimaryExpression, AdditiveExpression } type;
  union {
    PrimaryExpression primary_expression;
    AdditiveExpression additive_expression;
  };
  void Print(FILE *file);
};

struct BlockStatement {
  Token left_brace;
  StatementList statement_list;
  void Print(FILE *file);
};

struct Statement {
  enum struct Type { ExpressionStatement, BlockStatement } type;

  union {
    ExpressionStatement expression_statement;
    BlockStatement block_statement;
  };

  void Print(FILE *file);
};

struct AST {
  StatementList statements;

  void Print(FILE *file);
};

// - Building AST
// - Syntax analysis with meaningful messages
// - Printing AST
// - Each AST node is mapped to the source code location
class Parser {
  AST ast_;
  TokenIterator token_iterator_;

  StatementList parseStatementList(
      Token::Type stopper_token = Token::Type::End);

  Statement *parseStatement();
  Token parseStringLiteral();
  Token parseNumericLiteral();
  AdditiveExpression parseAdditiveExpression();
  Token eatToken(Token::Type type);

 public:
  Parser(const char *input) : token_iterator_(input), ast_() {}

  const AST &Run();
};
