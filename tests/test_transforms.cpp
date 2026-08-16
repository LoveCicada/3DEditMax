#include "test_harness.h"
#include "core/MatrixFormat.h"
#include "teach/Transforms.h"
#include <DirectXMath.h>
#include <cmath>
#include <cstring>

static bool near4(float a, float b) {
  return fabsf(a - b) < 1e-4f;
}

void runTransformTests() {
  TransformTRS id = transformIdentity();
  DirectX::XMMATRIX w = BuildWorld(id);
  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, w);
  TEST_CHECK(near4(m._11, 1.f) && near4(m._22, 1.f) && near4(m._33, 1.f) && near4(m._44, 1.f));

  TransformTRS t = transformIdentity();
  t.pos[0] = 2.f;
  DirectX::XMStoreFloat4x4(&m, BuildWorld(t));
  TEST_CHECK(near4(m._41, 2.f)); /* row-major store: translation in _41,_42,_43 */

  TeachingState s = teachingStateDefault();
  DirectX::XMMATRIX P = BuildProjection(s, 1.f);
  (void)P;
  TEST_CHECK(s.nearZ < s.farZ);

  DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
  TrackResult tr = TrackPoint(DirectX::XMFLOAT3(1.f, 2.f, 3.f), I, I, I);
  TEST_CHECK(near4(tr.world.x, 1.f) && near4(tr.world.y, 2.f) && near4(tr.world.z, 3.f));
  TEST_CHECK(near4(tr.ndc.x, 1.f) && near4(tr.ndc.w, 1.f));

  /* formatMatrix4: XMStoreFloat4x4 layout, no extra transpose */
  DirectX::XMFLOAT4X4 stored;
  DirectX::XMStoreFloat4x4(&stored, DirectX::XMMatrixIdentity());
  char lines[4][64];
  formatMatrix4(stored, MajorColumn, lines);
  TEST_CHECK(std::strcmp(lines[0], "1.000 0.000 0.000 0.000") == 0);
  TEST_CHECK(std::strcmp(lines[1], "0.000 1.000 0.000 0.000") == 0);
  TEST_CHECK(std::strcmp(lines[2], "0.000 0.000 1.000 0.000") == 0);
  TEST_CHECK(std::strcmp(lines[3], "0.000 0.000 0.000 1.000") == 0);
  formatMatrix4(stored, MajorRow, lines);
  TEST_CHECK(std::strcmp(lines[0], "1.000 0.000 0.000 0.000") == 0);

  TransformTRS tx = transformIdentity();
  tx.pos[0] = 2.f;
  DirectX::XMStoreFloat4x4(&stored, BuildWorld(tx));
  TEST_CHECK(near4(stored._41, 2.f));
  formatMatrix4(stored, MajorColumn, lines);
  TEST_CHECK(std::strcmp(lines[0], "1.000 0.000 0.000 2.000") == 0);
  TEST_CHECK(std::strcmp(lines[3], "0.000 0.000 0.000 1.000") == 0);
  formatMatrix4(stored, MajorRow, lines);
  TEST_CHECK(std::strcmp(lines[0], "1.000 0.000 0.000 0.000") == 0);
  TEST_CHECK(std::strcmp(lines[3], "2.000 0.000 0.000 1.000") == 0);

  /* viewport orbit / dolly (Task 10) */
  TeachingState cam = teachingStateDefault();
  applyOrbitDrag(&cam, 10.f, 5.f);
  TEST_CHECK(near4(cam.camYawDeg, 48.f));
  TEST_CHECK(near4(cam.camPitchDeg, 21.5f));
  cam.camPitchDeg = 88.f;
  applyOrbitDrag(&cam, 0.f, 20.f);
  TEST_CHECK(near4(cam.camPitchDeg, 89.f));
  cam.camPitchDeg = -88.f;
  applyOrbitDrag(&cam, 0.f, -20.f);
  TEST_CHECK(near4(cam.camPitchDeg, -89.f));
  cam.camDistance = 5.f;
  applyDollyWheel(&cam, 120);
  TEST_CHECK(near4(cam.camDistance, 4.5f));
  applyDollyWheel(&cam, -120);
  TEST_CHECK(near4(cam.camDistance, 4.95f));
  cam.camDistance = 0.5f;
  applyDollyWheel(&cam, 1);
  TEST_CHECK(near4(cam.camDistance, 0.5f));
  cam.camDistance = 50.f;
  applyDollyWheel(&cam, -1);
  TEST_CHECK(near4(cam.camDistance, 50.f));
}
