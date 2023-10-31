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
    case Token::Type::StringLiteral:
    case Token::Type::NumericLiteral: {
      result->type = Statement::Type::ExpressionStatement;
      result->expression_statement = parseExpressionStatement();
    } break;
    case Token::Type::LeftBrace: {
      const auto left_brace = eatToken(Token::Type::LeftBrace);
      result->type = Statement::Type::BlockStatement;
      result->block_statement = new BlockStatement{
          .left_brace = left_brace,
          .statement_list = parseStatementList(Token::Type::RightBrace)};
      eatToken(Token::Type::RightBrace);
    } break;
    default:
      throw std::runtime_error("Unimplemented");
      break;
  }
  return result;
}

ExpressionStatement *Parser::parseExpressionStatement() {
  const Token &lookahead = token_iterator_.Peek();
  ExpressionStatement *result = new ExpressionStatement{};
  switch (lookahead.type) {
    case Token::Type::StringLiteral:
    case Token::Type::NumericLiteral: {
      result->type = ExpressionStatement::Type::MultiplicativeExpression;
      result->multiplicative_expression =
          new MultiplicativeExpression{.left = parsePrimaryExpression()};
      default:
        throw std::runtime_error("Unimplemented");
    } break;
  }
  return result;
}

Token Parser::parseStringLiteral() {
  return eatToken(Token::Type::StringLiteral);
}

Token Parser::parseNumericLiteral() {
  return eatToken(Token::Type::NumericLiteral);
}

PrimaryExpression *Parser::parsePrimaryExpression() {
  const Token &lookahead = token_iterator_.Peek();
  PrimaryExpression *result = new PrimaryExpression{};
  switch (lookahead.type) {
    case Token::Type::StringLiteral: {
      result->type = PrimaryExpression::Type::StringLiteral;
      result->string_literal = parseStringLiteral();
    } break;
    case Token::Type::NumericLiteral: {
      result->type = PrimaryExpression::Type::NumericLiteral;
      result->string_literal = parseNumericLiteral();
    } break;
    default:
      throw std::runtime_error("Unimplemented");
  }
  return result;
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

AdditiveExpression *Parser::parseAdditiveExpression() { return {}; }

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
