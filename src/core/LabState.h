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
