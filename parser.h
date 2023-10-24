#pragma once
#include "tokenizer.h"
#include <vector>

struct Statement;

// TODO: replace with vector
struct StatementList {
  struct ListNode {
    Statement *statement;
    ListNode *next;
  };
  ListNode *head;
  ListNode *tail;

  StatementList &addStatement(Statement *s) {
    ListNode *new_node = new ListNode{.statement = s, .next = nullptr};
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
  enum struct Type { StringLiteral, NumericLiteral, BinaryExpression } type;
  union {
    Token string_literal;
    Token numeric_literal;
  };

  static ExpressionStatement stringLiteral(const char *string) {
    return {.type = Type::StringLiteral,
            .string_literal = {.type = Token::Type::StringLiteral,
                               .string = string}};
  }

  static ExpressionStatement numericLiteral(double number) {
    return {.type = Type::NumericLiteral,
            .string_literal = {.type = Token::Type::NumericLiteral,
                               .number = number}};
  }
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

  static Statement stringLiteral(const char *string) {
    return {.type = Type::ExpressionStatement,
            .expression_statement = ExpressionStatement::stringLiteral(string)};
  }
  static Statement numericLiteral(double number) {
    return {.type = Type::ExpressionStatement,
            .expression_statement =
                ExpressionStatement::numericLiteral(number)};
  }

  static Statement blockStatement(std::vector<Statement *> statements,
                                  SourcePos pos) {
    auto res = Statement{
        .type = Type::BlockStatement,
        .block_statement = {
            .left_brace = {.type = Token::Type::LeftBrace, .pos = pos},
        }};
    for (const auto &stmt : statements) {
      res.block_statement.statement_list.addStatement(stmt);
    }
    return res;
  }

  static Statement *newStringLiteral(const char *string) {
    return new Statement(stringLiteral(string));
  }

  static Statement *newNumericLiteral(double number) {
    return new Statement(numericLiteral(number));
  }

  static Statement *newBlockStatement(std::vector<Statement *> statements,
                                      SourcePos pos) {
    return new Statement(blockStatement(statements, pos));
  }
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