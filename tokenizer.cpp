#include "tokenizer.h"
#include <charconv>
#include <stdexcept>

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

const Token &TokenIterator::next() {
  if (!isInput()) {
    current_token_.type = Token::Type::End;
    return current_token_;
  }
  bool repeat = true;

  while (repeat) {
    repeat = false;
    char c = input_[cursor_];
    switch (c) {
      case '+': {
        current_token_.type = Token::Type::Plus;
        current_token_.pos = current_pos_;
        ++cursor_;
        current_pos_.col += 1;
      } break;
      case '-': {
        current_token_.type = Token::Type::Minus;
        current_token_.pos = current_pos_;
        ++cursor_;
        current_pos_.col += 1;
      } break;
      case '*': {
        current_token_.type = Token::Type::Multiply;
        current_token_.pos = current_pos_;
        ++cursor_;
        current_pos_.col += 1;
      } break;
      case '/': {
        current_token_.type = Token::Type::Divide;
        current_token_.pos = current_pos_;
        ++cursor_;
        current_pos_.col += 1;
      } break;
      case '{': {
        current_token_.type = Token::Type::LeftBrace;
        current_token_.pos = current_pos_;
        ++cursor_;
        current_pos_.col += 1;
      } break;
      case '}': {
        current_token_.type = Token::Type::RightBrace;
        current_token_.pos = current_pos_;
        ++cursor_;
        current_token_.pos.col += 1;
      } break;
      case CASE_DIGIT: {
        double v;
        const char *start = input_ + cursor_;
        const auto [end, _] = std::from_chars(start, input_ + input_length_, v);
        current_token_ = {
            .type = Token::NumericLiteral, .number = v, .pos = current_pos_};
        const size_t len = end - start;
        cursor_ += len;
        current_pos_.col += len;
      } break;
      case '"':
      case '\'': {
        current_token_.pos = current_pos_;
        const size_t start = cursor_++;
        current_pos_.col += 1;
        while (isInput() && input_[cursor_] != c) {
          ++cursor_;
          current_pos_.col += 1;
        }
        if (input_[cursor_] != c) {
          throw std::runtime_error("Unexpected end of stream.");
        }

        const size_t len = cursor_ - start - 1;
        char *string = new char[len + 1];
        string[len] = 0;
        memcpy(string, input_ + start + 1, len);
        current_token_.type = Token::StringLiteral;
        current_token_.string = string;
        ++cursor_;
        current_pos_.col += 1;
      } break;
      case '\n': {
        current_pos_.col = 1;
        current_pos_.line += 1;
        [[fallthrough]];
      }
      case '\r': {
        current_pos_.col -= 1;
        [[fallthrough]];
      }
      case ' ':
      case '\t': {
        cursor_ += 1;
        current_pos_.col += 1;
        repeat = true;
      } break;
      default:
        break;
    }
  }
  // FIXME:
  // 1. current_token_ is not set to Token::Type::End if the input exhausted
  // 2. if explicitly set to Token::Type::End, the last token will be lost
  return current_token_;
}
