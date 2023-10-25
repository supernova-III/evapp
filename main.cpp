#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "tokenizer.cpp"
#include "parser.cpp"

#include <gtest/gtest.h>

size_t getFileSize(FILE *file) {
  fseek(file, 0, SEEK_END);
  const size_t result = ftell(file);
  rewind(file);
  return result;
}

char *readEntireFile(const char *path, size_t &s) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    throw std::runtime_error("Cannot open source file");
  }

  const size_t size = getFileSize(file);
  char *res = (char *)calloc(size + 1, 1);
  if (!res) {
    throw std::runtime_error("Cannot allocate memory");
  }

  const auto read_res = fread(res, 1, size, file);
  if (read_res != size) {
    throw std::runtime_error("Cannot read the file");
  }
  s = size;

  return res;
}

int main(int argc, char **argv) {
  size_t size = 0;
  const char *content = readEntireFile("blocks_literals.eva", size);
  auto parser = Parser(content);
  auto ast = parser.Run();
  const auto *expr = ast.statements.head;
  FILE *out = fopen("blocks_literals.json", "w+");
  if (!out) {
    throw std::runtime_error("Cannot open the file");
  }
  Print(ast, out);
  fclose(out);
  // testing::InitGoogleTest();
  // const auto res = RUN_ALL_TESTS();
  return 0;
}

TEST(AST, Tests) {
  auto parser = Parser(R"(
    12
    "asd"
    { 
      13
      "asdasd"
    }

  )");
  auto ast = parser.Run();
  // clang-format off
  AST ref_ast = {
    .statements = {
      new Statement {
        .type = Statement::Type::ExpressionStatement,
        .expression_statement = {
          .type = ExpressionStatement::Type::NumericLiteral,
          .numeric_literal = Token::NumericLiteral(12)
        }
      },
      new Statement {
        .type = Statement::Type::ExpressionStatement,
        .expression_statement = {
          .type = ExpressionStatement::Type::StringLiteral,
          .string_literal = Token::StringLiteral("asd")
        }
      },
      new Statement {
        .type = Statement::Type::BlockStatement,
        .block_statement = {
          .statement_list = {
            new Statement {
              .type = Statement::Type::ExpressionStatement,
              .expression_statement = {
                .type = ExpressionStatement::Type::NumericLiteral,
                .numeric_literal = Token::NumericLiteral(12)
              }
            },
            new Statement {
              .type = Statement::Type::ExpressionStatement,
              .expression_statement = {
                .type = ExpressionStatement::Type::StringLiteral,
                .string_literal = Token::StringLiteral("asd")
              }
            }
          }
        }
      }
    }
  };
  // clang-format on
  ASSERT_EQ(ast, ref_ast);
}

TEST(Tokenizer, NumberTests) {
  struct Test {
    const char *input;
    Token expected;
    const char *desc;
  };
  Test inputs[] = {
      {.input = "123",
       .expected = Token{.type = Token::Type::NumericLiteral, .number = 123},
       .desc = "Integer"},
      {.input = "123.123",
       .expected =
           Token{.type = Token::Type::NumericLiteral, .number = 123.123},
       .desc = "Float"},
      {.input = "123.123e123",
       .expected =
           Token{.type = Token::Type::NumericLiteral, .number = 123.123e123},
       .desc = "Float with exponent"},
      {.input = "123.e-123",
       .expected =
           Token{.type = Token::Type::NumericLiteral, .number = 123.e-123},
       .desc = "Float with exponent"},
  };

  for (const auto &i : inputs) {
    TokenIterator iter(i.input);
    printf("Running %s...\n", i.desc);
    ASSERT_EQ(iter.Next(), i.expected);
  }
}
