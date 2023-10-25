#pragma once
#include "tokenizer.h"
#include <vector>

struct Statement;

// TODO: replace with vector
struct StatementList {
  struct Node {
    Statement *statement;
    Node *next;
  };
  Node *head = nullptr;
  Node *tail = nullptr;

  StatementList(std::initializer_list<Statement *> statements = {}) {
    for (const auto &statement : statements) {
      addStatement(statement);
    }
  }

  StatementList &addStatement(Statement *s) {
    Node *new_node = new Node{.statement = s, .next = nullptr};
    if (!head) {
      head = new_node;
      head->next = tail;
      tail = head;
      tail->next = nullptr;
    } else {
      tail->next = new_node;
      tail = new_node;
    }
    return *this;
  }
};

struct ExpressionStatement {
  enum struct Type { StringLiteral, NumericLiteral } type;
  union {
    Token string_literal;
    Token numeric_literal;
  };
};

struct BlockStatement {
  Token left_brace;
  StatementList statement_list;
};

struct Statement {
  enum struct Type { ExpressionStatement, BlockStatement } type;

  union {
    ExpressionStatement expression_statement;
    BlockStatement block_statement;
  };
};

struct AST {
  StatementList statements;
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