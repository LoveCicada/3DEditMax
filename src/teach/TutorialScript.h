#pragma once
#include "core/TeachingState.h"

struct TutorialStep {
  const char* title;
  const char* body;
  TeachingState state;
};

int tutorialStepCount();
TutorialStep tutorialStepAt(int index);
