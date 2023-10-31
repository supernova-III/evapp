#pragma once
#include "tokenizer.h"
#include <initializer_list>

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
};

struct ExpressionStatement;
struct PrimaryExpression;

struct MultiplicativeExpression {
  PrimaryExpression *left;
  // nullptr if the expression is just a primary expression
  PrimaryExpression *right;
  // Token::Type::End if the expression is just a primary expression
  Token multiplicative_operator;
};

struct AdditiveExpression {
  Token additive_operator;
  MultiplicativeExpression *left;
  MultiplicativeExpression *right;
};

struct PrimaryExpression {
  enum struct Type { StringLiteral, NumericLiteral } type;
  union {
    Token string_literal;
    Token numeric_literal;
  };
};

struct ExpressionStatement {
  enum struct Type { MultiplicativeExpression, AdditiveExpression } type;
  union {
    AdditiveExpression *additive_expression;
    MultiplicativeExpression *multiplicative_expression;
  };
};

struct BlockStatement {
  Token left_brace;
  StatementList statement_list;
};

struct Statement {
  enum struct Type { ExpressionStatement, BlockStatement } type;

  union {
    ExpressionStatement *expression_statement;
    BlockStatement *block_statement;
  };
};

struct AST {
  StatementList statements;
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
  ExpressionStatement *parseExpressionStatement();
  Token parseStringLiteral();
  Token parseNumericLiteral();
  PrimaryExpression *parsePrimaryExpression();
  AdditiveExpression *parseAdditiveExpression();
  MultiplicativeExpression *parseMultiplicativeExpression();
  Token eatToken(Token::Type type);

 public:
  Parser(const char *input) : token_iterator_(input), ast_() {}

  const AST &Run();
};
