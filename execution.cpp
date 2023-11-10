#include "defines.h"
#include "execution.h"

Object Evaluate(ExpressionStatement* statement) {
  switch (statement->type) {
    case ExpressionStatement::Type_NumericLiteral: {
      return {.type = Object::Type_Number,
              .number = statement->numeric_literal.literal.number};
    }
    case ExpressionStatement::Type_StringLiteral: {
      return {.type = Object::Type_String,
              .string = statement->string_literal.literal.string};
    }
    case ExpressionStatement::Type_Binary: {
      Object left = Evaluate(statement->binary.left);
      Object right = Evaluate(statement->binary.right);
      if (right.type != Object::Type_Number &&
          left.type != Object::Type_Number) {
        Panic("Invalid binary operation.");
      }
      switch (statement->binary.op.type) {
        case Token::Type_Plus:
          return {.type = Object::Type_Number,
                  .number = left.number + right.number};
        case Token::Type_Minus:
          return {.type = Object::Type_Number,
                  .number = left.number - right.number};
        case Token::Type_Multiply:
          return {.type = Object::Type_Number,
                  .number = left.number * right.number};
        case Token::Type_Divide:
          return {.type = Object::Type_Number,
                  .number = left.number / right.number};
        default:
          Panic("Invalid binary operator.");
      }
    }
    default:
      Panic("Unimplemented error");
  }
  return {};
}
