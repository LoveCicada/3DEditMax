#pragma once
#include <DirectXMath.h>

enum MajorOrder {
  MajorColumn = 0,
  MajorRow = 1
};

void formatMatrix4(const DirectX::XMFLOAT4X4& m, MajorOrder order, char out[4][64]);
