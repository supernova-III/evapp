#pragma once
#include "parser.h"

// Two kinds of execution:
// 1. Bytecode interpreter
// 2. AST-walker
struct Object {
  enum Type : uint8_t { Type_Number, Type_String } type;
  union {
    double number;
    const char* string;
  };
};

Object Evaluate(ExpressionStatement* statement);