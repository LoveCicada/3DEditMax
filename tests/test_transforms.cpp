#include "test_harness.h"
#include "core/MatrixFormat.h"
#include "teach/MeshBuild.h"
#include "teach/Transforms.h"
#include <DirectXMath.h>
#include <cmath>
#include <cstring>
#include <vector>

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

  /* Task 2: camera target + screen-space pan */
  TeachingState defCam = teachingStateDefault();
  TEST_CHECK(near4(defCam.camTarget[0], 0.f) && near4(defCam.camTarget[1], 0.f) &&
             near4(defCam.camTarget[2], 0.f));

  const float originT[3] = {0.f, 0.f, 0.f};
  const float pitch20 = DirectX::XMConvertToRadians(20.f);
  const float yaw45 = DirectX::XMConvertToRadians(45.f);
  DirectX::XMMATRIX Vorg = BuildView(5.f, 20.f, 45.f, originT);
  DirectX::XMVECTOR eyeOrg = DirectX::XMVectorSet(
      5.f * sinf(yaw45) * cosf(pitch20),
      5.f * sinf(pitch20),
      5.f * cosf(yaw45) * cosf(pitch20),
      1.f);
  DirectX::XMMATRIX Vexp = DirectX::XMMatrixLookAtLH(
      eyeOrg, DirectX::XMVectorZero(), DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f));
  DirectX::XMFLOAT4X4 gotV;
  DirectX::XMFLOAT4X4 expV;
  DirectX::XMStoreFloat4x4(&gotV, Vorg);
  DirectX::XMStoreFloat4x4(&expV, Vexp);
  TEST_CHECK(near4(gotV._11, expV._11) && near4(gotV._22, expV._22) &&
             near4(gotV._33, expV._33) && near4(gotV._41, expV._41) &&
             near4(gotV._42, expV._42) && near4(gotV._43, expV._43));

  const float tgt[3] = {1.f, 2.f, 3.f};
  DirectX::XMMATRIX Vt = BuildView(5.f, 0.f, 0.f, tgt);
  DirectX::XMVECTOR at = DirectX::XMVectorSet(1.f, 2.f, 3.f, 1.f);
  DirectX::XMVECTOR viewAt = DirectX::XMVector4Transform(at, Vt);
  TEST_CHECK(near4(DirectX::XMVectorGetX(viewAt), 0.f));
  TEST_CHECK(near4(DirectX::XMVectorGetY(viewAt), 0.f));
  TEST_CHECK(near4(DirectX::XMVectorGetZ(viewAt), 5.f));

  TeachingState pan = teachingStateDefault();
  pan.camPitchDeg = 0.f;
  pan.camYawDeg = 0.f;
  pan.camDistance = 10.f;
  applyPanDrag(&pan, 100.f, 0.f, 200.f, 200.f);
  TEST_CHECK(near4(pan.camTarget[0], 5.f));
  TEST_CHECK(near4(pan.camTarget[1], 0.f));
  TEST_CHECK(near4(pan.camTarget[2], 0.f));
  applyPanDrag(&pan, 0.f, 40.f, 200.f, 200.f);
  TEST_CHECK(near4(pan.camTarget[0], 5.f));
  TEST_CHECK(near4(pan.camTarget[1], 2.f));
  TEST_CHECK(near4(pan.camTarget[2], 0.f));

  TeachingState axisT = teachingStateDefault();
  axisT.objects[0].trs.pos[0] = 0.f;
  axisT.objects[0].trs.pos[1] = 0.f;
  axisT.objects[0].trs.pos[2] = 0.f;
  applyAxisTranslateDrag(&axisT, 0, 1.5f);
  TEST_CHECK(near4(axisT.objects[0].trs.pos[0], 1.5f));
  applyAxisTranslateDrag(&axisT, 1, -0.5f);
  TEST_CHECK(near4(axisT.objects[0].trs.pos[1], -0.5f));
  {
    TeachingState hitT = teachingStateDefault();
    hitT.camPitchDeg = 0.f;
    hitT.camYawDeg = 0.f;
    hitT.camDistance = 10.f;
    hitT.camTarget[0] = 0.f;
    hitT.camTarget[1] = 0.f;
    hitT.camTarget[2] = 0.f;
    hitT.objects[0].trs.pos[0] = 0.f;
    hitT.objects[0].trs.pos[1] = 0.f;
    hitT.objects[0].trs.pos[2] = 0.f;
    float ox = 0.f;
    float oy = 0.f;
    float ex = 0.f;
    float ey = 0.f;
    TEST_CHECK(projectWorldToScreen(hitT, 0.f, 0.f, 0.f, 200.f, 200.f, &ox, &oy));
    TEST_CHECK(projectWorldToScreen(hitT, worldAxisGizmoLength(), 0.f, 0.f, 200.f, 200.f, &ex, &ey));
    TEST_CHECK(hitWorldAxisHandle(hitT, (ox + ex) * 0.5f, (oy + ey) * 0.5f, 200.f, 200.f) == 0);
    TEST_CHECK(hitWorldAxisHandle(hitT, 2.f, 2.f, 200.f, 200.f) < 0);
    const float drag = axisTranslateFromDrag(hitT, 0, ex - ox, ey - oy, 200.f, 200.f);
    TEST_CHECK(near4(drag, worldAxisGizmoLength()));
  }

  /* Task 11: CPU mesh builders */
  TEST_CHECK(cubeVertexCount() > 0);
  std::vector<MeshVertex> cubeV;
  std::vector<unsigned short> cubeI;
  buildCube(&cubeV, &cubeI);
  TEST_CHECK(cubeI.size() == 36);
  TEST_CHECK(cubeV.size() == static_cast<size_t>(cubeVertexCount()));

  /* default / preset tracker point is a MeshBuild cube corner (±0.5) */
  TeachingState track = teachingStateDefault();
  bool onCorner = false;
  for (size_t vi = 0; vi < cubeV.size(); ++vi) {
    if (near4(cubeV[vi].px, track.trackModel[0]) &&
        near4(cubeV[vi].py, track.trackModel[1]) &&
        near4(cubeV[vi].pz, track.trackModel[2])) {
      onCorner = true;
      break;
    }
  }
  TEST_CHECK(onCorner);
  DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
  TrackResult corner = TrackPoint(
      DirectX::XMFLOAT3(track.trackModel[0], track.trackModel[1], track.trackModel[2]),
      ident, ident, ident);
  TEST_CHECK(near4(corner.world.x, 0.5f) && near4(corner.world.y, 0.5f) &&
             near4(corner.world.z, 0.5f));

  const int slices = 32;
  const int stacks = 24;
  std::vector<MeshVertex> sphereV;
  std::vector<unsigned short> sphereI;
  buildSphere(&sphereV, &sphereI, slices, stacks);
  TEST_CHECK(static_cast<int>(sphereV.size()) == (stacks + 1) * (slices + 1));
  TEST_CHECK(sphereV.size() > 0);
  TEST_CHECK(sphereI.size() > 0);

  std::vector<MeshEdge> cubeEdges;
  buildSilhouetteEdges(cubeV, cubeI, &cubeEdges, 1.f);
  TEST_CHECK(cubeEdges.size() == 12);

  std::vector<MeshEdge> sphereEdges;
  buildSilhouetteEdges(sphereV, sphereI, &sphereEdges, 1.f);
  TEST_CHECK(sphereEdges.size() > 12);
  TEST_CHECK(sphereEdges.size() < sphereI.size() / 2);

  std::vector<MeshVertex> cylV;
  std::vector<unsigned short> cylI;
  buildCylinder(&cylV, &cylI, 24);
  TEST_CHECK(cylV.size() > 0);
  TEST_CHECK(cylI.size() > 0);

  std::vector<MeshVertex> coneV;
  std::vector<unsigned short> coneI;
  buildCone(&coneV, &coneI, 0.15f, 0.36f, 8);
  TEST_CHECK(coneV.size() > 0);
  TEST_CHECK(coneI.size() > 0);
}
