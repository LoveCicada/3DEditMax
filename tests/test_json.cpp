#include "test_harness.h"
#include "teach/JsonIo.h"
#include <cmath>
#include <string>

static bool nearf(float a, float b) {
  return fabsf(a - b) < 1e-4f;
}

void runJsonTests() {
  TeachingState s = teachingStateDefault();
  std::string json;
  TEST_CHECK(teachingToJson(s, &json) == true);
  TEST_CHECK(json.empty() == false);

  TeachingState out;
  TEST_CHECK(teachingFromJson(json, &out) == true);
  TEST_CHECK(nearf(out.camDistance, s.camDistance));
  TEST_CHECK(nearf(out.objects[0].trs.pos[0], s.objects[0].trs.pos[0]));

  TeachingState missing;
  TEST_CHECK(teachingFromJson(std::string("{}"), &missing) == false);
  TEST_CHECK(teachingToJson(s, 0) == false);
  TEST_CHECK(teachingFromJson(json, 0) == false);
}
