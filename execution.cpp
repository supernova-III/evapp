#include "defines.h"
#include "execution.h"
#include "memory.h"
#include <unordered_map>

struct Namespace {
  Namespace* parent;
  std::unordered_map<std::string_view, Object> data = {};
};

static Namespace* currentNamespace = {};

Object namespaceLookup(const char* name, const Namespace& ns) {
  const Namespace* n = &ns;
  while (n != nullptr) {
    if (n->data.find(std::string_view(name)) != n->data.end()) {
      return n->data.at(std::string_view(name));
    }
    n = n->parent;
  }
  Panic("Identifier undeclared: %s", name);
  return {};
}

Object Evaluate(ExpressionStatement* statement) {
  switch (statement->type) {
    case ExpressionStatement::Type_Identifier: {
      // Namespace lookup goes here!
      return {.type = Object::Type_Identifier,
              .identifier = statement->identifier.name.string};
    }
    case ExpressionStatement::Type_Assignment: {
      // Getting value of the left side. It must be an identifier.
      Object left = Evaluate(statement->assignment.left);
      if (left.type != Object::Type_Identifier) {
        Panic("Invalid assignment. The left hand side must be an identifier.");
      }
      // Getting the right side. It must be a number for now.
      Object right = Evaluate(statement->assignment.right);
      if (right.type != Object::Type_Number) {
        Panic("Invalid assignment. The right hand side must be a number.");
      }

      currentNamespace->data[std::string_view(left.identifier)] = right;
      return right;
    }
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
      if (left.type == Object::Type_Identifier) {
        left = namespaceLookup(left.identifier, *currentNamespace);
      }
      Object right = Evaluate(statement->binary.right);
      if (right.type == Object::Type_Identifier) {
        right = namespaceLookup(right.identifier, *currentNamespace);
      }
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
    // The lastest evaluated Object in a block is a result
    case ExpressionStatement::Type_Block: {
      auto new_namespace = new Namespace{};
      new_namespace->parent = currentNamespace;
      currentNamespace = new_namespace;
      ExpressionStatement* node = statement->block.head;
      Object res = {};
      while (node != nullptr) {
        res = Evaluate(node);
        node = node->next;
      }
      return res;
    }
    default:
      Panic("Unimplemented error");
  }
  return {};
}
