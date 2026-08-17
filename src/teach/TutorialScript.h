#pragma once
#include "core/TeachingState.h"

struct TutorialStep {
  const char* title;
  const char* body;
  TeachingState state;
};

struct DemoMatrixFocus {
  bool w;
  bool v;
  bool p;
  bool mvp;
};

int tutorialStepCount();
TutorialStep tutorialStepAt(int index);
DemoMatrixFocus demoMatrixFocus(int tutorialStep, bool demoPlaying);
