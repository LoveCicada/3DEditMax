#pragma once
#include "core/TeachingState.h"
#include "core/LabState.h"

struct StateSnapshot {
  TeachingState teaching;
  LabState lab;
  int viewportW;
  int viewportH;
};

inline StateSnapshot stateSnapshotDefault() {
  StateSnapshot s;
  s.teaching = teachingStateDefault();
  s.lab = labStateDefault();
  s.viewportW = 1;
  s.viewportH = 1;
  return s;
}
