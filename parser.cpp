#include "parser.h"
#include "defines.h"
#include "memory.h"
#include <charconv>
#include <cstdint>
#include <cstdio>
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


static BlockAllocator expressionStatementAllocator =
    BlockAllocator(sizeof(ExpressionStatement), 256);

template <typename T>
T* newObject(T&& prototype, BlockAllocator& allocator) {
  T* result = static_cast<T*>(allocator.AllocateBlock());
  *result = std::move(prototype);
  return result;
}

static ExpressionStatement* newExpressionStatement(
    ExpressionStatement&& prototype) {
  return newObject<ExpressionStatement>(std::move(prototype),
                                        expressionStatementAllocator);
}

static BlockAllocator tokenAllocator = BlockAllocator(sizeof(Token), 512);

static Token* newToken(Token&& proto) {
  return newObject<Token>(std::move(proto), tokenAllocator);
}

class PermanentAllocator {
  void* memory_;
  uint32_t size_;
  uint32_t capacity_;

 public:
  PermanentAllocator(uint32_t capacity)
      : capacity_(capacity), size_(), memory_(::operator new(capacity)) {}

  void* Allocate(uint32_t size) {
    if (size_ + size >= capacity_) {
      Panic("Permanent allocator failed to allocate another %lu bytes.");
    }
    void* result = (char*)memory_ + size_;
    size_ += size;
    return result;
  }
};

static PermanentAllocator stringAllocator = PermanentAllocator(4 * 1024 * 1024);

char* newString(size_t string_size) {
  char* result = (char*)stringAllocator.Allocate(string_size + 1);
  result[string_size] = 0;
  return result;
}

const Token& TokenIterator::Next() {
  bool repeat = true;

  while (repeat) {
    if (!isInput()) {
      current_token_.type = Token::Type_End;
      return current_token_;
    }
    repeat = false;
    char c = input_[cursor_];
    switch (c) {
      case '=': {
        current_token_.type = Token::Type_Assign;
        ++cursor_;
      } break;
      case '+': {
        current_token_.type = Token::Type_Plus;
        ++cursor_;
      } break;
      case '-': {
        current_token_.type = Token::Type_Minus;
        ++cursor_;
      } break;
      case '*': {
        current_token_.type = Token::Type_Multiply;
        ++cursor_;
      } break;
      case '/': {
        current_token_.type = Token::Type_Divide;
        ++cursor_;
      } break;
      case '{': {
        current_token_.type = Token::Type_LeftBrace;
        ++cursor_;
      } break;
      case '}': {
        current_token_.type = Token::Type_RightBrace;
        ++cursor_;
      } break;
      case CASE_DIGIT: {
        double v;
        const char* start = input_ + cursor_;
        const auto [end, _] = std::from_chars(start, input_ + input_length_, v);
        current_token_ = {
            .type = Token::Type_NumericLiteral,
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
        char* string = newString(len);
        string[len] = 0;
        memcpy(string, input_ + start + 1, len);
        current_token_.type = Token::Type_StringLiteral;
        current_token_.string = string;
        ++cursor_;
      } break;
      case CASE_ALPHA: {
        const size_t start = cursor_++;
        while (isAlphaNum(input_[cursor_])) {
          ++cursor_;
        }
        const size_t len = cursor_ - start;
        current_token_.type = Token::Type_Identifier;
        const auto it =
            identifiers_.find(std::string_view(input_ + start, len));
        if (it != identifiers_.end()) {
          current_token_.string = it->data();
        } else {
          char* string = newString(len);
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

ExpressionStatement* Parser::statementList(Token starter, Token::Type stopper) {
  ExpressionStatement* list = newExpressionStatement({
      .type = ExpressionStatement::Type_Block,
      .block = {.starter = starter},
  });
  ExpressionStatement* current = list;
  current->block.head = expressionStatement();
  ExpressionStatement* node = current->block.head;
  while (token_iterator_.Peek().type != stopper) {
    ExpressionStatement* expression = expressionStatement();
    node->next = expression;
    node = expression;
  }
  return list;
}

ExpressionStatement* Parser::assignment() {
  ExpressionStatement* additive = additiveExpression();
  if (token_iterator_.Peek().type == Token::Type_Assign) {
    consumeToken(Token::Type_Assign);
    ExpressionStatement* right = additiveExpression();
    return newExpressionStatement({
        .type = ExpressionStatement::Type_Assignment,
        .assignment = {.left = additive, .right = right},
    });
  }
  return additive;
}

ExpressionStatement* Parser::numericLiteral() {
  return newExpressionStatement({
      .type = ExpressionStatement::Type_NumericLiteral,
      .numeric_literal = {.literal = consumeToken(Token::Type_NumericLiteral)},
  });
}

static bool isAddOp(Token::Type type) {
  return type == Token::Type_Plus || type == Token::Type_Minus;
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
  return type == Token::Type_Divide || type == Token::Type_Multiply;
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
    case Token::Type_Identifier:
      return assignment();
    case Token::Type_StringLiteral:
      return newExpressionStatement({
          .type = ExpressionStatement::Type_StringLiteral,
          .string_literal = {.literal =
                                 consumeToken(Token::Type_StringLiteral)},
      });
    case Token::Type_NumericLiteral:
      return additiveExpression();
    case Token::Type_LeftBrace: {
      const auto starter = consumeToken(Token::Type_LeftBrace);
      const auto statement_list =
          statementList(starter, Token::Type_RightBrace);
      consumeToken(Token::Type_RightBrace);
      return statement_list;
    } break;
    default:
      Panic("Unexpected token.");
  }
  return nullptr;
}

ExpressionStatement* Parser::primaryStatement() {
  switch (token_iterator_.Peek().type) {
    case Token::Type_NumericLiteral: {
      return numericLiteral();
    } break;
    case Token::Type_Identifier: {
      return newExpressionStatement({
          .type = ExpressionStatement::Type_Identifier,
          .identifier = {.name = consumeToken(token_iterator_.Peek().type)},
      });
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

ExpressionStatement* Parser::Run() {
  token_iterator_.Next();
  return statementList(Token{.type = Token::Type_Invalid});
}

ExpressionStatement* ExpressionStatement::Duplicate() {
  ExpressionStatement* result = newExpressionStatement({.type = type});
  switch (type) {
    case Type_StringLiteral: {
      result->string_literal.literal = string_literal.literal;
    } break;
    case Type_NumericLiteral: {
      result->numeric_literal.literal = numeric_literal.literal;
    } break;
    case Type_Block: {
      result->block = block;
      result->next = next;
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
    } break;
  }
  return result;
}

void ExpressionStatement::DumpJsonToFile(FILE* file) {
  switch (type) {
    case Type_Identifier: {
      fprintf(file, R"("Identifier":{"name":"%s"})", identifier.name.string);
    } break;
    case Type_StringLiteral: {
      fprintf(file, R"("StringLiteral":{"value":"%s"})",
              string_literal.literal.string);
    } break;
    case Type_NumericLiteral: {
      fprintf(file, R"("NumericLiteral":{"value":%f})",
              numeric_literal.literal.number);
    } break;
    case Type_Binary: {
      fprintf(file, R"("BinaryOperation":{)");
      fprintf(file, R"("Left":{)");
      binary.left->DumpJsonToFile(file);
      fprintf(file, "},");
      fprintf(file, R"("Operation":)");
      switch (binary.op.type) {
        case Token::Type_Plus:
          fprintf(file, R"("+")");
          break;
        case Token::Type_Minus:
          fprintf(file, R"("-")");
          break;
        case Token::Type_Multiply:
          fprintf(file, R"("*")");
          break;
        case Token::Type_Divide:
          fprintf(file, R"("/")");
          break;
        default:
          Panic("Invalid AST. Wrong operand in the binary operation.");
      }
      fprintf(file, ",");
      fprintf(file, R"("Right":{)");
      binary.right->DumpJsonToFile(file);
      fprintf(file, "}");
      fprintf(file, "}");
    } break;
    case Type_Assignment: {
      fprintf(file, R"("Assignment":{)");
      assignment.left->DumpJsonToFile(file);
      fprintf(file, ",");
      assignment.right->DumpJsonToFile(file);
      fprintf(file, "}");
    } break;
    case Type_Block: {
      if (block.starter.type == Token::Type_Invalid) {
        fprintf(file, R"([)");
      } else {
        fprintf(file, R"("Block":{"position":{"line":0,"col":0},"list":[)");
      }
      ExpressionStatement* statement = block.head;
      while (statement != nullptr) {
        fprintf(file, "{");
        statement->DumpJsonToFile(file);
        statement = statement->next;
        fprintf(file, "}");
        if (statement != nullptr) {
          fprintf(file, ",");
        }
      }
      fprintf(file, "]");
      if (block.starter.type != Token::Type_Invalid) {
        fprintf(file, "}");
      }
    } break;
  }
}
