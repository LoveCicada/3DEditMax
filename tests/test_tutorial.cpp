#include "test_harness.h"
#include "teach/TutorialScript.h"
#include "teach/DemoPlayer.h"
#include <cmath>

static bool nearf(float a, float b) {
  return fabsf(a - b) < 1e-4f;
}

void runTutorialTests() {
  TEST_CHECK(tutorialStepCount() == 8);

  TutorialStep s0 = tutorialStepAt(0);
  TEST_CHECK(s0.title != 0);
  TEST_CHECK(s0.body != 0);

  TutorialStep world = tutorialStepAt(1);
  TEST_CHECK(nearf(world.state.objects[0].trs.yawDeg, 35.f));
  TEST_CHECK(world.state.tutorialStep == 1);

  TutorialStep view = tutorialStepAt(2);
  TEST_CHECK(nearf(view.state.camPitchDeg, 40.f));
  TEST_CHECK(view.state.tutorialStep == 2);

  DemoPlayer demo;
  TeachingState from = teachingStateDefault();
  demo.start(from);
  TeachingState io = from;
  TEST_CHECK(demo.tick(0.f, &io) == true);
  TEST_CHECK(io.tutorialStep == 0);
  TEST_CHECK(io.demoPlaying == true);
  TEST_CHECK(demo.tick(2.f, &io) == true);
  TEST_CHECK(io.tutorialStep == 1);
  TEST_CHECK(demo.tick(14.f, &io) == false);
  TEST_CHECK(io.tutorialStep == 7);
  TEST_CHECK(io.demoPlaying == false);
}
