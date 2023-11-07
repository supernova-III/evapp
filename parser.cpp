#include "parser.h"
#include "defines.h"
#include <charconv>
#include <cstring>

#define CASE_DIGIT \
  '0' : case '1':  \
  case '2':        \
  case '3':        \
  case '4':        \
  case '5':        \
  case '6':        \
  case '7':        \
  case '8':        \
  case '9'

// clang-format off
#define CASE_ALPHA \
  'a': case 'b': case 'c': case 'd': \
  case 'e': case 'f': case 'g': case 'h': \
  case 'i': case 'j': case 'k': case 'l': \
  case 'm': case 'n': case 'o': case 'p': \
  case 'q': case 'r': case 's': case 't': \
  case 'u': case 'v': case 'w': case 'x': \
  case 'y': case 'z': case 'A': case 'B': \
  case 'C': case 'D': case 'E': case 'F': \
  case 'G': case 'H': case 'I': case 'J': \
  case 'K': case 'L': case 'M': case 'N': \
  case 'O': case 'P': case 'Q': case 'R': \
  case 'S': case 'T': case 'U': case 'V': \
  case 'W': case 'X': case 'Y': case 'Z'
// clang-format on

static bool isAlphaNum(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_';
}

const Token& TokenIterator::Next() {
  bool repeat = true;

  while (repeat) {
    if (!isInput()) {
      current_token_.type = Token::Type::End;
      return current_token_;
    }
    repeat = false;
    char c = input_[cursor_];
    switch (c) {
      case '=': {
        current_token_.type = Token::Type::Assign;
        ++cursor_;
      } break;
      case '+': {
        current_token_.type = Token::Type::Plus;
        ++cursor_;
      } break;
      case '-': {
        current_token_.type = Token::Type::Minus;
        ++cursor_;
      } break;
      case '*': {
        current_token_.type = Token::Type::Multiply;
        ++cursor_;
      } break;
      case '/': {
        current_token_.type = Token::Type::Divide;
        ++cursor_;
      } break;
      case '{': {
        current_token_.type = Token::Type::LeftBrace;
        ++cursor_;
      } break;
      case '}': {
        current_token_.type = Token::Type::RightBrace;
        ++cursor_;
      } break;
      case CASE_DIGIT: {
        double v;
        const char* start = input_ + cursor_;
        const auto [end, _] = std::from_chars(start, input_ + input_length_, v);
        current_token_ = {
            .type = Token::Type::NumericLiteral,
            .number = v,
        };
        const size_t len = end - start;
        cursor_ += len;
      } break;
      case '"':
      case '\'': {
        const size_t start = cursor_++;
        while (isInput() && input_[cursor_] != c) {
          ++cursor_;
        }
        if (input_[cursor_] != c) {
          Panic("Unexpected EOF");
        }

        const size_t len = cursor_ - start - 1;
        char* string = new char[len + 1];
        string[len] = 0;
        memcpy(string, input_ + start + 1, len);
        current_token_.type = Token::Type::StringLiteral;
        current_token_.string = string;
        ++cursor_;
      } break;
      case CASE_ALPHA: {
        const size_t start = cursor_++;
        while (isAlphaNum(input_[cursor_])) {
          ++cursor_;
        }
        const size_t len = cursor_ - start;
        current_token_.type = Token::Type::Identifier;
        const auto it =
            identifiers_.find(std::string_view(input_ + start, len));
        if (it != identifiers_.end()) {
          current_token_.string = it->data();
        } else {
          char* string = new char[len + 1];
          string[len] = 0;
          memcpy(string, input_ + start, len);
          current_token_.string = string;
        }
      } break;
      case '\n':
      case '\r':
      case ' ':
      case '\t': {
        cursor_ += 1;
        repeat = true;
      } break;
      default:
        break;
    }
  }
  return current_token_;
}

StatementList& StatementList::Push(ExpressionStatement* new_statement) {
  if (head != nullptr) {
    tail->next = new StatementList::Node{
        .expression_statement = new_statement,
        .next = nullptr,
    };
    tail = tail->next;
  } else {
    head = new StatementList::Node{
        .expression_statement = new_statement,
        .next = tail,
    };
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

ExpressionStatement* Parser::assignment() {
  ExpressionStatement* additive = additiveExpression();
  if (token_iterator_.Peek().type == Token::Type::Assign) {
    consumeToken(Token::Type::Assign);
    ExpressionStatement* right = additiveExpression();
    return new ExpressionStatement{
        .type = ExpressionStatement::Type_Assignment,
        .assignment = {.left = additive, .right = right},
    };
  }
  return additive;
}

ExpressionStatement* Parser::numericLiteral() {
  return new ExpressionStatement{
      .type = ExpressionStatement::Type_NumericLiteral,
      .numeric_literal = {.literal = consumeToken(Token::Type::NumericLiteral)},
  };
}

static bool isAddOp(Token::Type type) {
  return type == Token::Type::Plus || type == Token::Type::Minus;
}

ExpressionStatement* Parser::additiveExpression() {
  ExpressionStatement* left = multiplicativeExpression();
  while (isAddOp(token_iterator_.Peek().type)) {
    Token operation = token_iterator_.Peek();
    token_iterator_.Next();
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
  ExpressionStatement* left = primaryStatement();
  while (isMulOp(token_iterator_.Peek().type)) {
    Token operation = token_iterator_.Peek();
    token_iterator_.Next();
    ExpressionStatement* right = primaryStatement();
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
    case Token::Type::Identifier:
      return assignment();
    case Token::Type::StringLiteral:
      return new ExpressionStatement{
          .type = ExpressionStatement::Type_StringLiteral,
          .string_literal = {.literal =
                                 consumeToken(Token::Type::StringLiteral)},
      };
    case Token::Type::NumericLiteral:
      return additiveExpression();
    case Token::Type::LeftBrace: {
      const auto starter = consumeToken(Token::Type::LeftBrace);
      const auto statement_list = statementList(Token::Type::RightBrace);
      consumeToken(Token::Type::RightBrace);
      return new ExpressionStatement{
          .type = ExpressionStatement::Type_Block,
          .block = {.starter = starter, .list = statement_list},
      };
    } break;
    default:
      Panic("Unexpected token.");
  }
  return nullptr;
}

ExpressionStatement* Parser::primaryStatement() {
  switch (token_iterator_.Peek().type) {
    case Token::Type::NumericLiteral: {
      return numericLiteral();
    } break;
    case Token::Type::Identifier: {
      return new ExpressionStatement{
          .type = ExpressionStatement::Type_Identifier,
          .identifier = {.name = consumeToken(token_iterator_.Peek().type)},
      };
    }
    default:
      Panic("Unexpected token");
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
    case Type_Assignment: {
      result->assignment.left = assignment.left;
      result->assignment.right = assignment.right;
    } break;
    case Type_Identifier: {
      result->identifier.name = identifier.name;
    }
  }
  return result;
}
