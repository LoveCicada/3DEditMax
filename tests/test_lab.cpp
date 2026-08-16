#include "test_harness.h"
#include "core/LabState.h"
#include <cstring>

void runLabTests() {
  LabState s = labStateDefault();
  TEST_CHECK(s.fillMode == 3);
  TEST_CHECK(s.cullMode == 3);
  TEST_CHECK(s.depthEnable);
  TEST_CHECK(std::strcmp(s.shaderVariant, "unlit") == 0);

  TEST_CHECK(labEffectiveFillMode(3, false) == 3);
  TEST_CHECK(labEffectiveFillMode(2, false) == 2);
  TEST_CHECK(labEffectiveFillMode(3, true) == 2);
  TEST_CHECK(labEffectiveFillMode(2, true) == 2);

  TEST_CHECK(labEffectiveCullMode(1) == 1);
  TEST_CHECK(labEffectiveCullMode(2) == 2);
  TEST_CHECK(labEffectiveCullMode(3) == 3);
  TEST_CHECK(labEffectiveCullMode(99) == 3);

  TEST_CHECK(labShadeModeX("unlit", 0) == 0.f);
  TEST_CHECK(labShadeModeX("normal", 0) == 1.f);
  TEST_CHECK(labShadeModeX("checker", 0) == 2.f);
  TEST_CHECK(labShadeModeX("unlit", 1) == 1.f);
  TEST_CHECK(labShadeModeX("unlit", 2) == 2.f);
}
