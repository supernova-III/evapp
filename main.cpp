#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "tokenizer.cpp"
#include "parser.cpp"

#include <gtest/gtest.h>

TEST(Parser, Parser) {
  auto parser = Parser(R"""(
      {
        123
        'asd'
        { 124 'asv' }
      }
    )""");
  auto ast = parser.run();
}

TEST(Tokenizer, NumberTests) {
  struct Test {
    const char *input;
    Token expected;
    const char *desc;
  };
  Test inputs[] = {
      {.input = "123",
       .expected = Token{.type = Token::NumericLiteral, .number = 123},
       .desc = "Integer"},
      {.input = "123.123",
       .expected = Token{.type = Token::NumericLiteral, .number = 123.123},
       .desc = "Float"},
      {.input = "123.123e123",
       .expected = Token{.type = Token::NumericLiteral, .number = 123.123e123},
       .desc = "Float with exponent"},
      {.input = "123.e-123",
       .expected = Token{.type = Token::NumericLiteral, .number = 123.e-123},
       .desc = "Float with exponent"},
  };

  for (const auto &i : inputs) {
    TokenIterator iter(i.input);
    printf("Running %s...\n", i.desc);
    ASSERT_EQ(iter.next(), i.expected);
  }
}
