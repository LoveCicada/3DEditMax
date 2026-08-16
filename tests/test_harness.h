#pragma once
#include <cstdio>

int& testFailureCount();

#define TEST_CHECK(cond) do { \
  if (!(cond)) { \
    std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    ++testFailureCount(); \
  } \
} while (0)
