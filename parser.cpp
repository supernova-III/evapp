#include "parser.h"
#include "defines.h"

StatementList& StatementList::Push(ExpressionStatement* new_statement) {
  if (head != nullptr) {
    tail->next = new StatementList::Node{.expression_statement = new_statement,
                                         .next = nullptr};
    tail = tail->next;
  } else {
    head = new StatementList::Node{.expression_statement = new_statement,
                                   .next = tail};
    tail = head;
  }
  return *this;
}

StatementList Parser::statementList(Token::Type stopper) {
  StatementList result = {};
  while (token_iterator_.Peek().type != stopper) {
    ExpressionStatement* expression = expressionStatement();
    result.Push(expression);
  }
  return result;
}

ExpressionStatement* Parser::numericLiteral() {
  return new ExpressionStatement{
      .type = ExpressionStatement::Type_NumericLiteral,
      .numeric_literal =
          {
              .literal = consumeToken(Token::Type::NumericLiteral),
          },
  };
}

static bool isAddOp(Token::Type type) {
  return type == Token::Type::Plus || type == Token::Type::Minus;
}
ExpressionStatement* Parser::additiveExpression() {
  ExpressionStatement* left = multiplicativeExpression();
  Token operation = token_iterator_.Peek();
  while (isAddOp(operation.type)) {
    operation = token_iterator_.Next();
    ExpressionStatement* right = multiplicativeExpression();
    left->binary.left = left->Duplicate();
    left->binary.right = right;
    left->binary.op = operation;
    left->type = ExpressionStatement::Type_Binary;
  }
  return left;
}

static bool isMulOp(Token::Type type) {
  return type == Token::Type::Divide || type == Token::Type::Multiply;
}

ExpressionStatement* Parser::multiplicativeExpression() {
  ExpressionStatement* left = numericLiteral();
  Token operation = token_iterator_.Peek();
  while (isMulOp(operation.type)) {
    operation = token_iterator_.Next();
    ExpressionStatement* right = numericLiteral();
    left->binary.left = left->Duplicate();
    left->binary.right = right;
    left->binary.op = operation;
    left->type = ExpressionStatement::Type_Binary;
  }
  return left;
}

ExpressionStatement* Parser::expressionStatement() {
  Token token = token_iterator_.Peek();
  switch (token.type) {
    case Token::Type::StringLiteral:
      return new ExpressionStatement{
          .type = ExpressionStatement::Type_StringLiteral,
          .string_literal =
              {
                  .literal = consumeToken(Token::Type::StringLiteral),
              },
      };
    case Token::Type::NumericLiteral:
      return additiveExpression();
    case Token::Type::LeftBrace: {
      const auto starter = consumeToken(Token::Type::LeftBrace);
      const auto statement_list = statementList(Token::Type::RightBrace);
      consumeToken(Token::Type::RightBrace);
      return new ExpressionStatement{
          .type = ExpressionStatement::Type_Block,
          .block =
              {
                  .starter = starter,
                  .list = statement_list,
              },
      };
    } break;
    default:
      Panic("Unexpected token.");
  }
  return nullptr;
}

Token Parser::consumeToken(Token::Type token_type) {
  Token token = token_iterator_.Peek();
  if (token.type != token_type) {
    Panic("Unexpected token.");
  }
  token_iterator_.Next();
  return token;
}

StatementList Parser::Run() {
  token_iterator_.Next();
  return statementList();
}

ExpressionStatement* ExpressionStatement::Duplicate() {
  ExpressionStatement* result = new ExpressionStatement{.type = type};
  switch (type) {
    case Type_StringLiteral: {
      result->string_literal.literal = string_literal.literal;
    } break;
    case Type_NumericLiteral: {
      result->numeric_literal.literal = numeric_literal.literal;
    } break;
    case Type_Block: {
      result->block = block;
    } break;
    case Type_Binary: {
      result->binary.op = binary.op;
      result->binary.left = binary.left;
      result->binary.right = binary.right;
    } break;
  }
  return result;
}
