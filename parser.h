#pragma once
#include "tokenizer.h"

struct Statement;

struct StatementList {
  struct ListNode {
    Statement *statement;
    ListNode *next;
  };
  ListNode *head;
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