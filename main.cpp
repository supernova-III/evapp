#include <stdio.h>
#include <stddef.h>
#include "parser.h"
#include "defines.h"
#include "execution.h"

namespace {
size_t getFileSize(FILE *file) {
  fseek(file, 0, SEEK_END);
  const size_t result = ftell(file);
  rewind(file);
  return result;
}

char *readEntireFile(const char *path) {
  FILE *file = NULL;
  fopen_s(&file, path, "rb");
  if (file == NULL) {
    Panic("Cannot open file %s", path);
  }

  const size_t size = getFileSize(file);
  char *res = new char[size + 1];
  res[size] = 0;

  const auto read_res = fread(res, 1, size, file);
  if (read_res != size) {
    Panic("Cannot read file %s", path);
  }

  return res;
}
}  // namespace

int main(int argc, char **argv) {
  const char *content = readEntireFile("blocks_literals.eva");
  auto parser = Parser(content);
  const auto tree = parser.Run();
  auto node = tree->block.head;
  while (node != nullptr) {
    Object o = Evaluate(node);
    switch (o.type) {
      case Object::Type_String: printf("> %s\n", o.string); break;
      case Object::Type_Number: printf("> %f\n", o.number); break;
    }
    node = node->next;
  }
  // FILE *file = NULL;
  // const char *path = "block_literals.json";
  // fopen_s(&file, "block_literals.json", "wb");
  // if (file == NULL) {
    // Panic("Cannot open file %s", path);
  // }
  // tree->DumpJsonToFile(file);
  return 0;
}