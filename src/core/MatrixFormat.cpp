#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include "core/MatrixFormat.h"
#include <cstdio>

void formatMatrix4(const DirectX::XMFLOAT4X4& m, MajorOrder order, char out[4][64]) {
  for (int line = 0; line < 4; ++line) {
    float a = 0.f;
    float b = 0.f;
    float c = 0.f;
    float d = 0.f;
    if (order == MajorColumn) {
      a = m.m[0][line];
      b = m.m[1][line];
      c = m.m[2][line];
      d = m.m[3][line];
    } else {
      a = m.m[line][0];
      b = m.m[line][1];
      c = m.m[line][2];
      d = m.m[line][3];
    }
    std::sprintf(out[line], "%.3f %.3f %.3f %.3f",
                 static_cast<double>(a),
                 static_cast<double>(b),
                 static_cast<double>(c),
                 static_cast<double>(d));
  }
}
