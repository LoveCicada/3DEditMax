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

  /* Task 2: camTarget round-trip; missing key stays origin */
  TeachingState withT = teachingStateDefault();
  withT.camTarget[0] = 1.25f;
  withT.camTarget[1] = -2.5f;
  withT.camTarget[2] = 3.f;
  std::string withJson;
  TEST_CHECK(teachingToJson(withT, &withJson) == true);
  TeachingState withOut;
  TEST_CHECK(teachingFromJson(withJson, &withOut) == true);
  TEST_CHECK(nearf(withOut.camTarget[0], 1.25f));
  TEST_CHECK(nearf(withOut.camTarget[1], -2.5f));
  TEST_CHECK(nearf(withOut.camTarget[2], 3.f));

  std::string noTargetJson;
  TEST_CHECK(teachingToJson(teachingStateDefault(), &noTargetJson) == true);
  const std::string camKey = "\"camTarget\":";
  std::string::size_type keyPos = noTargetJson.find(camKey);
  if (keyPos != std::string::npos) {
    std::string::size_type close = noTargetJson.find(']', keyPos);
    TEST_CHECK(close != std::string::npos);
    std::string::size_type eraseEnd = close + 1;
    if (eraseEnd < noTargetJson.size() && noTargetJson[eraseEnd] == ',') {
      ++eraseEnd;
    }
    std::string::size_type eraseStart = keyPos;
    if (eraseEnd == close + 1) {
      std::string::size_type comma = noTargetJson.rfind(',', keyPos);
      if (comma != std::string::npos) {
        eraseStart = comma;
      }
    }
    noTargetJson.erase(eraseStart, eraseEnd - eraseStart);
  }
  TEST_CHECK(noTargetJson.find(camKey) == std::string::npos);
  TeachingState noTargetOut;
  noTargetOut.camTarget[0] = 9.f;
  noTargetOut.camTarget[1] = 9.f;
  noTargetOut.camTarget[2] = 9.f;
  TEST_CHECK(teachingFromJson(noTargetJson, &noTargetOut) == true);
  TEST_CHECK(nearf(noTargetOut.camTarget[0], 0.f));
  TEST_CHECK(nearf(noTargetOut.camTarget[1], 0.f));
  TEST_CHECK(nearf(noTargetOut.camTarget[2], 0.f));
}
