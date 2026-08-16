#include "test_harness.h"

int& testFailureCount() {
  static int n = 0;
  return n;
}

int main() {
  TEST_CHECK(1 == 1);
  if (testFailureCount() != 0) {
    return 1;
  }
  return 0;
}
