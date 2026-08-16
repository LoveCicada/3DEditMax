#pragma once
#include <cmath>

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

enum MeshId {
  MeshCube = 0,
  MeshSphere = 1,
  MeshCylinder = 2
};

enum LayoutMode {
  LayoutOne = 0,
  LayoutThree = 1
};

enum ShadingMode {
  ShadeSolid = 0,
  ShadeNormal = 1,
  ShadeChecker = 2,
  ShadeWire = 3
};

enum ProjMode {
  ProjPerspective = 0,
  ProjOrtho = 1
};

struct TransformTRS {
  float pos[3];
  float pitchDeg;
  float yawDeg;
  float rollDeg;
  float scale[3];
};

struct TeachingObject {
  MeshId mesh;
  TransformTRS trs;
};

struct TeachingState {
  TeachingObject objects[3];
  LayoutMode layout;
  ShadingMode shading;
  float camDistance;
  float camPitchDeg;
  float camYawDeg;
  float camTarget[3];
  ProjMode proj;
  float fovDeg;
  float aspect;
  bool aspectFollowViewport;
  float nearZ;
  float farZ;
  float trackModel[3];
  int tutorialStep;
  bool demoPlaying;
};

inline TransformTRS transformIdentity() {
  TransformTRS t;
  t.pos[0] = 0.f; t.pos[1] = 0.f; t.pos[2] = 0.f;
  t.pitchDeg = 0.f; t.yawDeg = 0.f; t.rollDeg = 0.f;
  t.scale[0] = 1.f; t.scale[1] = 1.f; t.scale[2] = 1.f;
  return t;
}

inline void applyOrbitDrag(TeachingState* t, float dx, float dy) {
  t->camYawDeg += dx * 0.3f;
  t->camPitchDeg += dy * 0.3f;
  if (t->camPitchDeg > 89.f) {
    t->camPitchDeg = 89.f;
  }
  if (t->camPitchDeg < -89.f) {
    t->camPitchDeg = -89.f;
  }
  while (t->camYawDeg > 180.f) {
    t->camYawDeg -= 360.f;
  }
  while (t->camYawDeg < -180.f) {
    t->camYawDeg += 360.f;
  }
}

inline void applyPanDrag(TeachingState* t, float dx, float dy, float viewportW,
                         float viewportH) {
  const float denom =
      (viewportH > 1.f) ? viewportH : ((viewportW > 1.f) ? viewportW : 1.f);
  const float scale = t->camDistance / denom;
  const float kDeg = 3.14159265f / 180.f;
  const float pitch = t->camPitchDeg * kDeg;
  const float yaw = t->camYawDeg * kDeg;
  const float cp = cosf(pitch);
  const float sp = sinf(pitch);
  const float sy = sinf(yaw);
  const float cy = cosf(yaw);
  const float fx = -sy * cp;
  const float fy = -sp;
  const float fz = -cy * cp;
  float rx = fz;
  float ry = 0.f;
  float rz = -fx;
  float rlen = sqrtf(rx * rx + ry * ry + rz * rz);
  if (rlen < 1e-8f) {
    rx = cy;
    ry = 0.f;
    rz = -sy;
    rlen = sqrtf(rx * rx + rz * rz);
    if (rlen < 1e-8f) {
      rlen = 1.f;
    }
  }
  rx /= rlen;
  ry /= rlen;
  rz /= rlen;
  const float ux = fy * rz - fz * ry;
  const float uy = fz * rx - fx * rz;
  const float uz = fx * ry - fy * rx;
  const float moveR = dx * scale;
  const float moveU = -dy * scale;
  t->camTarget[0] += rx * moveR + ux * moveU;
  t->camTarget[1] += ry * moveR + uy * moveU;
  t->camTarget[2] += rz * moveR + uz * moveU;
}

inline void applyDollyWheel(TeachingState* t, int delta) {
  t->camDistance *= (delta > 0 ? 0.9f : 1.1f);
  if (t->camDistance < 0.5f) {
    t->camDistance = 0.5f;
  }
  if (t->camDistance > 50.f) {
    t->camDistance = 50.f;
  }
}

inline TeachingState teachingStateDefault() {
  TeachingState s;
  for (int i = 0; i < 3; ++i) {
    s.objects[i].mesh = MeshCube;
    s.objects[i].trs = transformIdentity();
    s.objects[i].trs.pos[0] = (i - 1) * 2.5f;
  }
  s.layout = LayoutOne;
  s.shading = ShadeSolid;
  s.camDistance = 5.f;
  s.camPitchDeg = 20.f;
  s.camYawDeg = 45.f;
  s.camTarget[0] = 0.f;
  s.camTarget[1] = 0.f;
  s.camTarget[2] = 0.f;
  s.proj = ProjPerspective;
  s.fovDeg = 60.f;
  s.aspect = 16.f / 9.f;
  s.aspectFollowViewport = true;
  s.nearZ = 0.1f;
  s.farZ = 100.f;
  s.trackModel[0] = 0.5f;
  s.trackModel[1] = 0.5f;
  s.trackModel[2] = 0.5f;
  s.tutorialStep = 0;
  s.demoPlaying = false;
  return s;
}
