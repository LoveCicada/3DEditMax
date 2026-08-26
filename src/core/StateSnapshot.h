#pragma once
#include "core/TeachingState.h"
#include "core/LabState.h"

struct StateSnapshot {
  TeachingState teaching;
  LabState lab;
  int viewportW;
  int viewportH;
  int gizmoHoverAxis;
  int gizmoActiveAxis;
};

inline StateSnapshot stateSnapshotDefault() {
  StateSnapshot s;
  s.teaching = teachingStateDefault();
  s.lab = labStateDefault();
  s.viewportW = 1;
  s.viewportH = 1;
  s.gizmoHoverAxis = -1;
  s.gizmoActiveAxis = -1;
  return s;
}
