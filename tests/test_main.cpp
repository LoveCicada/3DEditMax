#include "test_harness.h"

void runQueueTests();
void runRenderThreadTests();
void runTransformTests();

int& testFailureCount() {
  static int n = 0;
  return n;
}

int main() {
  TEST_CHECK(1 == 1);
  runQueueTests();
  runRenderThreadTests();
  runTransformTests();
  if (testFailureCount() != 0) {
    return 1;
  }
  return 0;
}
