#include "parser.h"

StatementList Parser::parseStatementList(Token::Type stopper_token) {
  StatementList result = {};
  result.head = new StatementList::ListNode{};
  result.head->statement = parseStatement();
  auto *node = result.head;
  while (token_iterator_ && token_iterator_.peek().type != stopper_token) {
    auto *new_node = new StatementList::ListNode{};
    new_node->statement = parseStatement();
    node->next = new_node;
    node = new_node;
  }
  return result;
}

Statement *Parser::parseStatement() {
  const Token &lookahead = token_iterator_.peek();
  Statement *result = new Statement{};

  switch (lookahead.type) {
  case Token::Type::StringLiteral: {
    result->type = Statement::Type::ExpressionStatement;
    result->expression_statement.type =
        ExpressionStatement::Type::StringLiteral;
    result->expression_statement.string_literal = parseStringLiteral();
  } break;
  case Token::Type::NumericLiteral: {
    result->type = Statement::Type::ExpressionStatement;
    result->expression_statement.type =
        ExpressionStatement::Type::NumericLiteral;
    result->expression_statement.numeric_literal = parseNumericLiteral();
  } break;
  case Token::Type::LeftBrace: {
    eatToken(Token::Type::LeftBrace);
    result->type = Statement::Type::BlockStatement;
    result->block_statement.statement_list =
        parseStatementList(Token::Type::RightBrace);
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
  Token res = token_iterator_.peek();
  if (res.type != type) {
    throw std::runtime_error("Unexpected token");
  }
  token_iterator_.next();
  return res;
}

const AST &Parser::run() {
  token_iterator_.next();
  ast_.statements = parseStatementList();
  return ast_;
}

void print(const ExpressionStatement &expression_statement) {
  printf(R"({"type":"ExpressionStatement","expression":{"type":)");
  switch (expression_statement.type) {
  case ExpressionStatement::Type::StringLiteral: {
    printf(R"("StringLiteral",)");
    printf(R"("value":"%s")", expression_statement.string_literal.string);
  } break;
  case ExpressionStatement::Type::NumericLiteral: {
    printf(R"("NumericLiteral",)");
    printf(R"("value":%f)", expression_statement.numeric_literal.number);
  } break;
  }
  printf("}}");
}
