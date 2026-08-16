#pragma once
#include "core/TeachingState.h"

class DemoPlayer {
public:
  DemoPlayer();
  void start(const TeachingState& from);
  bool tick(float dtSec, TeachingState* io);

private:
  TeachingState m_from;
  float m_elapsed;
};
