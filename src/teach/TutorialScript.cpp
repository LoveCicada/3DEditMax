#include "teach/TutorialScript.h"

namespace {

TeachingState makeStepState(int index) {
  TeachingState s = teachingStateDefault();
  s.tutorialStep = index;
  s.demoPlaying = false;
  switch (index) {
    case 0:
      s.camDistance = 6.f;
      s.camPitchDeg = 15.f;
      s.camYawDeg = 25.f;
      break;
    case 1:
      s.objects[0].trs.yawDeg = 35.f;
      s.objects[0].trs.pitchDeg = 10.f;
      break;
    case 2:
      s.camPitchDeg = 40.f;
      s.camYawDeg = 20.f;
      break;
    case 3:
      s.fovDeg = 45.f;
      s.proj = ProjPerspective;
      break;
    case 4:
      s.objects[0].trs.yawDeg = 20.f;
      s.camYawDeg = 30.f;
      s.fovDeg = 55.f;
      break;
    case 5:
      s.nearZ = 1.f;
      s.farZ = 20.f;
      s.camDistance = 8.f;
      break;
    case 6:
      s.layout = LayoutThree;
      s.objects[0].mesh = MeshCube;
      s.objects[1].mesh = MeshSphere;
      s.objects[2].mesh = MeshCylinder;
      break;
    case 7:
      s.shading = ShadeNormal;
      break;
    default:
      break;
  }
  return s;
}

}  // namespace

int tutorialStepCount() {
  return 8;
}

TutorialStep tutorialStepAt(int index) {
  static const char* kTitle[8] = {
      "\xE5\x9D\x90\xE6\xA0\x87\xE7\xA9\xBA\xE9\x97\xB4",
      "World",
      "View",
      "Projection",
      "MVP",
      "\xE8\xBF\x91\xE8\xBF\x9C\xE5\xB9\xB3\xE9\x9D\xA2",
      "\xE4\xB8\x89\xE7\x89\xA9\xE4\xBD\x93\xE5\xAF\xB9\xE7\x85\xA7",
      "DX11 CB \xE5\xAF\xB9\xE7\x85\xA7",
  };
  static const char* kBody[8] = {
      "Model -> World -> View -> Clip. DX11 left-handed, Y up.",
      "World = Scale * Rotation * Translation. Object yaw = 35 deg.",
      "LookAt origin, +Y up. Camera pitch = 40 deg.",
      "Perspective FOV vs ortho. Change fovDeg and watch P.",
      "MVP = W * V * P. HLSL mul is column-major.",
      "nearZ / farZ set the frustum depth range.",
      "LayoutThree: cube, sphere, cylinder under one camera.",
      "Constant buffer uploads W/V/P/WVP to match the board.",
  };
  if (index < 0) {
    index = 0;
  }
  if (index > 7) {
    index = 7;
  }
  TutorialStep step;
  step.title = kTitle[index];
  step.body = kBody[index];
  step.state = makeStepState(index);
  return step;
}

DemoMatrixFocus demoMatrixFocus(int tutorialStep, bool demoPlaying) {
  DemoMatrixFocus f;
  f.w = false;
  f.v = false;
  f.p = false;
  f.mvp = false;
  if (!demoPlaying) {
    return f;
  }
  switch (tutorialStep) {
    case 0:
    case 4:
    case 7:
      f.w = true;
      f.v = true;
      f.p = true;
      f.mvp = true;
      break;
    case 1:
      f.w = true;
      f.mvp = true;
      break;
    case 2:
      f.v = true;
      f.mvp = true;
      break;
    case 3:
    case 5:
      f.p = true;
      f.mvp = true;
      break;
    default:
      break;
  }
  return f;
}
