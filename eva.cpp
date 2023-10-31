#include <stdio.h>
#include <stdint.h>
#include <charconv>

#define PANIC(format, ...)     \
  printf(format, __VA_ARGS__); \
  exit(1)
enum TokenType : uint8_t {
  TokenType_End,
  TokenType_NumericLiteral,
  TokenType_StringLiteral,
  TokenType_LeftBrace,
  TokenType_RightBrace,
  TokenType_Plus,
  TokenType_Minus,
  TokenType_Multiply,
  TokenType_Divide,
};

#define TOK_STRING(TOK) [TokenType_##TOK] = #TOK

static constexpr const char* token_string[] = {
    TOK_STRING(End),           TOK_STRING(NumericLiteral),
    TOK_STRING(StringLiteral), TOK_STRING(LeftBrace),
    TOK_STRING(RightBrace),    TOK_STRING(Plus),
    TOK_STRING(Minus),         TOK_STRING(Multiply),
    TOK_STRING(Divide),
};

static constexpr TokenType token_string_reverse[] = {
    ['+'] = TokenType_Plus,      ['-'] = TokenType_Minus,
    ['*'] = TokenType_Multiply,  ['/'] = TokenType_Divide,
    ['{'] = TokenType_LeftBrace, ['}'] = TokenType_RightBrace,
};

struct Token {
  TokenType type;
  union {
    // 0-terminated string
    const char* string_literal;
    double numeric_literal;
  };
  // An offset from the start of an input
  uint32_t offset;
};

struct TokenizerContext {
  const char* input;
  uint32_t input_size;
  uint32_t input_pos;
  Token current_token;
};

struct SourcePos {
  uint32_t line, col;
};

// Calculates source position of a token produced by a tokenizer. User must
// provide the line number and the offset from the previous token in the token
// stream. The behavior is undefined if the token is not produced by the
// tokenizer. Or prev_line_number and prev_offset don't correspond to the
// previous token in the token stream.
//
// It's recommended to use this function when you need to calculate source
// positions of the sequence of tokens as it's possible to iterate the entire
// input just once for all tokens in a stream.
SourcePos CalculateSourcePosRelative(const TokenizerContext* context,
                                     const Token* token,
                                     uint32_t prev_line_number,
                                     uint32_t prev_offset) {
  SourcePos result = {.line = prev_line_number, .col = 1};

  // A variable that will contain the very last \n character after iterating
  // over the input. It's needed to define the column
  uint32_t latest_nl_pos = 0;
  for (uint32_t i = prev_offset; i < token->offset; ++i) {
    if (context->input[i] == '\n') {
      result.line += 1;
      latest_nl_pos = i;
    }
  }

  // Current source position is on the next line after the last \n, but if
  // there's only one line, we have to take it into account
  result.line += (result.line > 1);
  // Current column is an offset of the token offset from the latest \n
  // character
  result.col = token->offset - latest_nl_pos;
  return result;
}

// Calculates source position of a token produced by a tokenizer. The behavior
// is undefined if the token is not produced by the tokenizer. It iterates the
// entire input until the given token is met. It's not recommended to use this
// function to find the source positions of several tokens as it will iterate
// the same input several times. This function is suitable for error/warning
// reporting, when the only one token required to locate the bad statement. This
// function IS NOT suitable for printing an AST
// It's not recommended to use this function
// to find the source positions of several tokens as it will iterate the same
// input several times.
// This function is suitable for error/warning reporting, when the only one
// token required to locate the bad statement. This function IS NOT suitable for
// printing an AST
SourcePos CalculateSourcePosAbsolute(const TokenizerContext* context,
                                     const Token* token) {
  return CalculateSourcePosRelative(context, token, 1, 0);
}

TokenizerContext InitTokenizer(const char* input, uint32_t input_size) {
  return {.input = input, .input_size = input_size, .input_pos = 0};
}

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

static bool isInput(TokenizerContext* ctx) {
  return ctx->input_pos < ctx->input_size;
}

Token* NextToken(TokenizerContext* ctx) {
  bool repeat = true;

  while (repeat) {
    if (!isInput(ctx)) {
      ctx->current_token.type = TokenType_End;
      return &ctx->current_token;
    }
    repeat = false;
    char c = ctx->input[ctx->input_pos];
    switch (c) {
      case '+':
      case '-':
      case '*':
      case '/':
      case '{':
      case '}': {
        ctx->current_token.type = token_string_reverse[c];
        ctx->current_token.offset = ctx->input_pos;
        ++ctx->input_pos;
      } break;
      case CASE_DIGIT: {
        double v;
        const char* start = ctx->input + ctx->input_pos;
        const auto [end, _] =
            std::from_chars(start, ctx->input + ctx->input_size, v);
        ctx->current_token = {.type = TokenType_NumericLiteral,
                              .numeric_literal = v,
                              .offset = ctx->input_pos};
        const size_t len = end - start;
        ctx->input_pos += len;
      } break;
      case '"':
      case '\'': {
        ctx->current_token.offset = ctx->input_pos;
        const size_t start = ctx->input_pos++;
        while (isInput(ctx) && ctx->input[ctx->input_pos] != c) {
          ++ctx->input_pos;
        }
        if (ctx->input[ctx->input_pos] != c) {
          const auto pos = CalculateSourcePosAbsolute(ctx, &ctx->current_token);
          PANIC("Unexpected end of stream at <%u, %u>", pos.line, pos.col);
        }

        const size_t len = ctx->input_pos - start - 1;
        char* string = new char[len + 1];
        string[len] = 0;
        memcpy(string, ctx->input + start + 1, len);
        ctx->current_token.type = TokenType_StringLiteral;
        ctx->current_token.string_literal = string;
        ++ctx->input_pos;
      } break;
      case '\n':
      case '\r':
      case ' ':
      case '\t': {
        ++ctx->input_pos;
        repeat = true;
      } break;
      default:
        break;
    }
  }
  return &ctx->current_token;
}

Token* PeekToken(TokenizerContext* ctx) { return &ctx->current_token; }

struct AstNode {
  // Subtrees
  AstNode* left;
  AstNode* right;

  union {
    Token block_start;
    Token numeric_literal;
    Token string_literal;
    Token op;
  } value;

  enum NodeType : uint8_t {
    // clang-format off
    NodeType_Glue,
    NodeType_Statement,
      NodeType_ExpressionStatement,
        NodeType_PrimaryExpression,
          NodeType_StringLiteral,
          NodeType_NumericLiteral,
        NodeType_AdditiveExpression,
      NodeType_BlockStatement,
    NodeType_StatementList,
      NodeType_StatementListEnd
    // clang-format on
  } type;
};

struct ParsingContext {
  AstNode ast;
  TokenizerContext tokenizer_context;
  Token lookahead;
};

ParsingContext InitParser(const char* input, uint32_t input_size) {
  return {.ast = {.type = AstNode::NodeType_StatementList},
          .tokenizer_context = InitTokenizer(input, input_size)};
}

namespace {
Token expectToken(ParsingContext* ctx, TokenType token_type) {
  Token token = *PeekToken(&ctx->tokenizer_context);
  if (token.type != token_type) {
    const auto pos =
        CalculateSourcePosAbsolute(&ctx->tokenizer_context, &ctx->lookahead);
    PANIC("Unexpected token at <%u, %u>: %s. Expected token of type '%s'",
          pos.line, pos.col, token_string[ctx->lookahead.type],
          token_string[token_type]);
  }
  ctx->lookahead = *NextToken(&ctx->tokenizer_context);
  return token;
}

AstNode* parseStatementList(ParsingContext* ctx,
                            TokenType stopper_token = TokenType_End);
AstNode* parseStatement(ParsingContext* ctx) {
  const Token* token = PeekToken(&ctx->tokenizer_context);
  switch (token->type) {
    case TokenType_StringLiteral: {
      const Token result_token = expectToken(ctx, TokenType_StringLiteral);
      auto result = new AstNode{.value = {.string_literal = result_token},
                                .type = AstNode::NodeType_StringLiteral};
      return result;

    } break;
    case TokenType_NumericLiteral: {
      const Token result_token = expectToken(ctx, TokenType_NumericLiteral);
      auto result = new AstNode{.value = {.numeric_literal = result_token},
                                .type = AstNode::NodeType_NumericLiteral};
      return result;
    } break;
    case TokenType_LeftBrace: {
      Token expected = expectToken(ctx, TokenType_LeftBrace);
      auto result = new AstNode{.value = {.block_start = expected},
                                .type = AstNode::NodeType_BlockStatement};
      result->left = parseStatementList(ctx, TokenType_RightBrace);
      expectToken(ctx, TokenType_RightBrace);
      return result;
    } break;
    default:
      NextToken(&ctx->tokenizer_context);
      break;
  }
  return nullptr;
}

AstNode* parseStatementList(ParsingContext* ctx, TokenType stopper_token) {
  AstNode* head = new AstNode{.type = AstNode::NodeType_StatementList};
  AstNode* node = head;

  while (PeekToken(&ctx->tokenizer_context)->type != stopper_token) {
    AstNode* new_node = parseStatement(ctx);
    node->left = new_node;
    node = new_node;
  }
  node->left = new AstNode{.type = AstNode::NodeType_StatementListEnd};
  return head;
}
}  // namespace

const AstNode* RunParser(ParsingContext* ctx) {
  ctx->lookahead = *NextToken(&ctx->tokenizer_context);
  return parseStatementList(ctx);
}

int main() {
  ParsingContext ctx = InitParser("{\"ASD\"\n123}", 11);
  const AstNode* ast = RunParser(&ctx);

  return 0;
}