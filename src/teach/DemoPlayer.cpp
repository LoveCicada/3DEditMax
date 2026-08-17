#include "teach/DemoPlayer.h"
#include "teach/TutorialScript.h"

static const float kSecPerStep = 2.f;

DemoPlayer::DemoPlayer()
    : m_from(teachingStateDefault())
    , m_elapsed(0.f) {
}

void DemoPlayer::start(const TeachingState&) {
  m_from = teachingStateDefault();
  m_elapsed = 0.f;
}

bool DemoPlayer::tick(float dtSec, TeachingState* io) {
  if (!io) {
    return false;
  }
  m_elapsed += dtSec;
  const int n = tutorialStepCount();
  if (n <= 0) {
    io->demoPlaying = false;
    return false;
  }
  int step = static_cast<int>(m_elapsed / kSecPerStep);
  if (step >= n) {
    *io = tutorialStepAt(n - 1).state;
    io->demoPlaying = false;
    return false;
  }
  *io = tutorialStepAt(step).state;
  io->demoPlaying = true;
  return true;
}
