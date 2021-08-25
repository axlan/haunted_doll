#pragma once

#include <Arduino.h>

// Custom strcpy, that returns dst + strlen.
// Note: the return ptr points to the NULL added to dst.
char * PrgmStrCpy(char* dst, const char* src) {
  for (size_t i = 0;; i++) {
    char c = pgm_read_byte(src + i);
    dst[i] = c;
    if (c == 0) {
      return dst + i;
    }
  }
  return nullptr;
}
