#include "tokenizer.h"
#include <charconv>
#include <stdexcept>

#define CASE_DIGIT                                                             \
  '0' : case '1':                                                              \
  case '2':                                                                    \
  case '3':                                                                    \
  case '4':                                                                    \
  case '5':                                                                    \
  case '6':                                                                    \
  case '7':                                                                    \
  case '8':                                                                    \
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
      const char *start = input_ + cursor_;
      const auto [end, _] = std::from_chars(start, input_ + input_length_, v);
      current_token_ = {.type = Token::NumericLiteral, .number = v};
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
        throw std::runtime_error("Unexpected end of stream.");
      }

      const size_t len = cursor_ - start - 1;
      char *string = new char[len + 1];
      string[len] = 0;
      memcpy(string, input_ + start + 1, len);
      current_token_.type = Token::StringLiteral;
      current_token_.string = string;
      ++cursor_;
    } break;
    case '\n':
    case ' ':
    case '\t': {
      cursor_ += 1;
      repeat = true;
    } break;
    default:
      break;
    }
  }
  if (!isInput()) {
    current_token_.type = Token::Type::End;
  }
  return current_token_;
}
