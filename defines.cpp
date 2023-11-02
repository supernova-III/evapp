#include "defines.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void Panic(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  printf("\n");
  va_end(args);
  exit(1);
}