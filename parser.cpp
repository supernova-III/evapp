#include "parser.h"

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

void Print(const ExpressionStatement &expression_statement, FILE *file) {
  fprintf(file, R"({"type":"ExpressionStatement","expression":{"type":)");
  switch (expression_statement.type) {
    case ExpressionStatement::Type::StringLiteral: {
      fprintf(
          file,
          R"("StringLiteral","source_pos":{"line":%llu,"col":%llu}, "value":"%s")",
          expression_statement.string_literal.pos.line,
          expression_statement.string_literal.pos.col,
          expression_statement.string_literal.string);
    } break;
    case ExpressionStatement::Type::NumericLiteral: {
      fprintf(
          file,
          R"("NumericLiteral","source_pos":{"line":%llu,"col":%llu},"value":%f)",
          expression_statement.numeric_literal.pos.line,
          expression_statement.numeric_literal.pos.col,
          expression_statement.numeric_literal.number);
    } break;
  }
  fprintf(file, "}}");
}

void Print(const BlockStatement &block_statement, FILE *file) {
  fprintf(
      file,
      R"({"type":"BlockStatement","source_pos":{"line":%llu,"col":%llu},"statement_list":)",
      block_statement.left_brace.pos.line, block_statement.left_brace.pos.col);
  Print(block_statement.statement_list, file);
  fprintf(file, "}");
}

void Print(const Statement &statement, FILE *file) {
  fprintf(file, R"({"type":"Statement","statement":)");
  switch (statement.type) {
    case Statement::Type::BlockStatement: {
      Print(statement.block_statement, file);
    } break;
    case Statement::Type::ExpressionStatement: {
      Print(statement.expression_statement, file);
    } break;
  }
  fprintf(file, "}");
}

void Print(const StatementList &statement_list, FILE *file) {
  fprintf(file, R"({"type":"StatementList","statement_list":[)");
  const auto *ptr = statement_list.head;
  while (ptr != nullptr) {
    if (ptr->statement) {
      Print(*ptr->statement, file);
    }
    if (ptr->next) {
      fprintf(file, ",");
    }
    ptr = ptr->next;
  }
  fprintf(file, "]}");
}

void Print(const AST &ast, FILE *file) {
  Print(ast.statements, file);
  fprintf(file, "\n");
}

bool StatementList::operator==(const StatementList &other) const noexcept {
  Node *current = head;
  Node *other_current = other.head;
  while (current != nullptr && other_current != nullptr) {
    if (*current->statement != *other_current->statement) {
      return false;
    }
    current = current->next;
    other_current = other_current->next;
  }
  if (current != nullptr || other_current != nullptr) {
    return false;
  }
  return true;
}

bool StatementList::operator!=(const StatementList &other) const noexcept {
  return !(*this == other);
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

bool ExpressionStatement::operator==(
    const ExpressionStatement &other) const noexcept {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case ExpressionStatement::Type::NumericLiteral: {
      return numeric_literal == other.numeric_literal;
    } break;
    case ExpressionStatement::Type::StringLiteral: {
      return string_literal == other.string_literal;
    } break;
  }
  return false;
}

bool ExpressionStatement::operator!=(
    const ExpressionStatement &other) const noexcept {
  return !((*this) == other);
}

bool BlockStatement::operator==(const BlockStatement &other) const noexcept {
  if (left_brace != other.left_brace) {
    return false;
  }
  return statement_list == other.statement_list;
}

bool BlockStatement::operator!=(const BlockStatement &other) const noexcept {
  return !(*this == other);
}

bool Statement::operator==(const Statement &other) const noexcept {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case Statement::Type::ExpressionStatement:
      return expression_statement == other.expression_statement;
    case Statement::Type::BlockStatement:
      return block_statement == other.block_statement;
  }
  return false;
}

bool Statement::operator!=(const Statement &other) const noexcept {
  return !(*this == other);
}

bool AST::operator==(const AST &other) const noexcept {
  return statements == other.statements;
}

bool AST::operator!=(const AST &other) const noexcept {
  return statements != other.statements;
}
