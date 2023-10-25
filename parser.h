#pragma once
#include "tokenizer.h"
#include <vector>

struct Statement;

struct StatementList {
  struct Node {
    Statement *statement;
    Node *next;
  };
  Node *head = nullptr;
  Node *tail = nullptr;

  bool operator==(const StatementList &other) const noexcept;
  bool operator!=(const StatementList &other) const noexcept;

  StatementList(std::initializer_list<Statement *> statements = {});
  StatementList &addStatement(Statement *s);
};

struct ExpressionStatement {
  enum struct Type { StringLiteral, NumericLiteral } type;
  union {
    Token string_literal;
    Token numeric_literal;
  };
  bool operator==(const ExpressionStatement &other) const noexcept;
  bool operator!=(const ExpressionStatement &other) const noexcept;
};

struct BlockStatement {
  Token left_brace;
  StatementList statement_list;
  bool operator==(const BlockStatement &other) const noexcept;
  bool operator!=(const BlockStatement &other) const noexcept;
};

struct Statement {
  enum struct Type { ExpressionStatement, BlockStatement } type;

  union {
    ExpressionStatement expression_statement;
    BlockStatement block_statement;
  };

  bool operator==(const Statement &other) const noexcept;
  bool operator!=(const Statement &other) const noexcept;
};

struct AST {
  StatementList statements;
  bool operator==(const AST &other) const noexcept;
  bool operator!=(const AST &other) const noexcept;
};

class Parser {
  AST ast_;
  TokenIterator token_iterator_;

  StatementList
  parseStatementList(Token::Type stopper_token = Token::Type::End);

  Statement *parseStatement();
  Token parseStringLiteral();
  Token parseNumericLiteral();
  Token eatToken(Token::Type type);

public:
  Parser(const char *input) : token_iterator_(input), ast_() {}

  const AST &run();
};

void print(const AST &ast, FILE *file);
void print(const ExpressionStatement &expression_statement, FILE *file);
void print(const StatementList &statement_list, FILE *file);
void print(const BlockStatement &block_statement, FILE *file);
void print(const Statement &statement, FILE *file);