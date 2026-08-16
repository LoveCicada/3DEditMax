#include "teach/DemoPlayer.h"

DemoPlayer::DemoPlayer()
    : m_from(teachingStateDefault())
    , m_elapsed(0.f) {
}

void DemoPlayer::start(const TeachingState& from) {
  m_from = from;
  m_elapsed = 0.f;
}

bool DemoPlayer::tick(float dtSec, TeachingState* io) {
  if (!io) {
    return false;
  }
  m_elapsed += dtSec;
  *io = m_from;
  if (m_elapsed >= 8.f) {
    io->camYawDeg = 0.f;
    io->demoPlaying = false;
    return false;
  }
  float yaw = 0.f;
  if (m_elapsed <= 4.f) {
    yaw = 90.f * (m_elapsed / 4.f);
  } else {
    yaw = 90.f * (1.f - (m_elapsed - 4.f) / 4.f);
  }
  io->camYawDeg = yaw;
  io->demoPlaying = true;
  return true;
}
