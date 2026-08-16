#pragma once
#include <cstring>

struct LabState {
  char shaderVariant[32];
  int fillMode;
  int cullMode;
  bool depthEnable;
  bool wantDebugLayer;
};

inline LabState labStateDefault() {
  LabState s;
  std::memset(s.shaderVariant, 0, sizeof(s.shaderVariant));
  const char* name = "unlit";
  for (int i = 0; name[i] && i < 31; ++i) {
    s.shaderVariant[i] = name[i];
  }
  s.fillMode = 3;  /* D3D11_FILL_SOLID */
  s.cullMode = 3;  /* D3D11_CULL_BACK */
  s.depthEnable = true;
  s.wantDebugLayer = true;
  return s;
}

inline void labSetShaderVariant(LabState* s, const char* name) {
  if (!s) {
    return;
  }
  std::memset(s->shaderVariant, 0, sizeof(s->shaderVariant));
  if (!name) {
    return;
  }
  for (int i = 0; name[i] && i < 31; ++i) {
    s->shaderVariant[i] = name[i];
  }
}

/* fillMode 2 = D3D11_FILL_WIREFRAME, 3 = D3D11_FILL_SOLID */
inline int labEffectiveFillMode(int labFillMode, bool teachingWire) {
  if (teachingWire || labFillMode == 2) {
    return 2;
  }
  return 3;
}

/* cullMode 1 = NONE, 2 = FRONT, 3 = BACK */
inline int labEffectiveCullMode(int labCullMode) {
  if (labCullMode == 1 || labCullMode == 2 || labCullMode == 3) {
    return labCullMode;
  }
  return 3;
}

/* teachingShading: 0 solid, 1 normal, 2 checker (ShadeWire does not change PS) */
inline float labShadeModeX(const char* variant, int teachingShading) {
  if (teachingShading == 1) {
    return 1.f;
  }
  if (teachingShading == 2) {
    return 2.f;
  }
  if (variant && std::strcmp(variant, "normal") == 0) {
    return 1.f;
  }
  if (variant && std::strcmp(variant, "checker") == 0) {
    return 2.f;
  }
  return 0.f;
}
