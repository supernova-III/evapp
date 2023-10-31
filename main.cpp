#include <stdio.h>
#include <stddef.h>
#include <stdexcept>
#include "parser.h"

size_t getFileSize(FILE *file) {
  fseek(file, 0, SEEK_END);
  const size_t result = ftell(file);
  rewind(file);
  return result;
}

char *readEntireFile(const char *path, size_t &s) {
  FILE *file = NULL;
  fopen_s(&file, path, "rb");
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
  return 0;
}