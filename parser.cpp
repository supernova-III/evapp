#include "parser.h"
#include <stdexcept>

StatementList Parser::parseStatementList(Token::Type stopper_token) {
  StatementList result = {};
  result.head = new StatementList::Node{};
  result.head->statement = parseStatement();
  auto *node = result.head;
  while (token_iterator_.Peek().type != stopper_token) {
    auto *new_node = new StatementList::Node{};
    new_node->statement = parseStatement();
    node->next = new_node;
    node = new_node;
  }
  return result;
}

Statement *Parser::parseStatement() {
  const Token &lookahead = token_iterator_.Peek();
  Statement *result = new Statement{};

  switch (lookahead.type) {
    case Token::Type::StringLiteral: {
      result->type = Statement::Type::ExpressionStatement;
      result->expression_statement.type =
          ExpressionStatement::Type::PrimaryExpression;
      result->expression_statement.primary_expression.type =
          PrimaryExpression::Type::Literal;
      result->expression_statement.primary_expression.literal.type =
          Literal::Type::String;
      result->expression_statement.primary_expression.literal.token =
          parseStringLiteral();
    } break;
    case Token::Type::NumericLiteral: {
      result->type = Statement::Type::ExpressionStatement;
      result->expression_statement.type =
          ExpressionStatement::Type::PrimaryExpression;
      result->expression_statement.primary_expression.type =
          PrimaryExpression::Type::Literal;
      result->expression_statement.primary_expression.literal.type =
          Literal::Type::Number;
      result->expression_statement.primary_expression.literal.token =
          parseNumericLiteral();
    } break;
    case Token::Type::LeftBrace: {
      const auto left_brace = eatToken(Token::Type::LeftBrace);
      result->type = Statement::Type::BlockStatement;
      result->block_statement.statement_list =
          parseStatementList(Token::Type::RightBrace);
      result->block_statement.left_brace = left_brace;
      eatToken(Token::Type::RightBrace);
    } break;
    default:
      throw std::runtime_error("Unimplemented");
      break;
  }
  return result;
}

Token Parser::parseStringLiteral() {
  return eatToken(Token::Type::StringLiteral);
}

Token Parser::parseNumericLiteral() {
  return eatToken(Token::Type::NumericLiteral);
}

Token Parser::eatToken(Token::Type type) {
  Token res = token_iterator_.Peek();
  if (res.type != type) {
    throw std::runtime_error("Unexpected token");
  }
  token_iterator_.Next();
  return res;
}

const AST &Parser::Run() {
  token_iterator_.Next();
  ast_.statements = parseStatementList();
  return ast_;
}

AdditiveExpression Parser::parseAdditiveExpression() { return {}; }

void ExpressionStatement::Print(FILE *file) {
  fprintf(file, R"({"type":"ExpressionStatement","expression":)");
  switch (type) {
    case ExpressionStatement::Type::PrimaryExpression: {
      primary_expression.Print(file);
    } break;
    case ExpressionStatement::Type::AdditiveExpression: {
      additive_expression.Print(file);
    } break;
  }
  fprintf(file, "}");
}

void BlockStatement::Print(FILE *file) {
  fprintf(
      file,
      R"({"type":"BlockStatement","source_pos":{"line":%llu,"col":%llu},"statement_list":)",
      left_brace.pos.line, left_brace.pos.col);
  statement_list.Print(file);
  fprintf(file, "}");
}

void Statement::Print(FILE *file) {
  fprintf(file, R"({"type":"Statement","statement":)");
  switch (type) {
    case Statement::Type::BlockStatement: {
      block_statement.Print(file);
    } break;
    case Statement::Type::ExpressionStatement: {
      expression_statement.Print(file);
    } break;
  }
  fprintf(file, "}");
}

void StatementList::Print(FILE *file) {
  fprintf(file, R"({"type":"StatementList","statement_list":[)");
  const auto *ptr = head;
  while (ptr != nullptr) {
    if (ptr->statement) {
      ptr->statement->Print(file);
    }
    if (ptr->next) {
      fprintf(file, ",");
    }
    ptr = ptr->next;
  }
  fprintf(file, "]}");
}

void AdditiveExpression::Print(FILE *file) { fprintf(file, "{}"); }

void PrimaryExpression::Print(FILE *file) {
  fprintf(file, R"({"type":"PrimaryExpression",)");
  switch (type) {
    case PrimaryExpression::Type::Literal: {
      fprintf(file, R"("literal":)");
      const auto item = literal;
      switch (item.type) {
        case Literal::Type::Number: {
          fprintf(file,
                  R"({"number":%f,"source_pos":{"line":%llu,"col":%llu}})",
                  item.token.number, item.token.pos.line, item.token.pos.col);
        } break;
        case Literal::Type::String: {
          fprintf(file,
                  R"({"string":"%s","source_pos":{"line":%llu,"col":%llu}})",
                  item.token.string, item.token.pos.line, item.token.pos.col);

        } break;
      }
    } break;
  }
  fprintf(file, "}");
}

void AST::Print(FILE *file) {
  statements.Print(file);
  fprintf(file, "\n");
}

StatementList::StatementList(std::initializer_list<Statement *> statements) {
  for (const auto &statement : statements) {
    AddStatement(statement);
  }
}

StatementList &StatementList::AddStatement(Statement *s) {
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
