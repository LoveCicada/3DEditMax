#include "teach/Transforms.h"

using namespace DirectX;

XMMATRIX BuildWorld(const TransformTRS& trs) {
  const XMMATRIX s = XMMatrixScaling(trs.scale[0], trs.scale[1], trs.scale[2]);
  const XMMATRIX r = XMMatrixRotationRollPitchYaw(
      XMConvertToRadians(trs.pitchDeg),
      XMConvertToRadians(trs.yawDeg),
      XMConvertToRadians(trs.rollDeg));
  const XMMATRIX t = XMMatrixTranslation(trs.pos[0], trs.pos[1], trs.pos[2]);
  return s * r * t;
}

XMMATRIX BuildView(float distance, float pitchDeg, float yawDeg) {
  const float pitch = XMConvertToRadians(pitchDeg);
  const float yaw = XMConvertToRadians(yawDeg);
  const XMVECTOR eye = XMVectorSet(
      distance * sinf(yaw) * cosf(pitch),
      distance * sinf(pitch),
      distance * cosf(yaw) * cosf(pitch),
      1.f);
  const XMVECTOR at = XMVectorZero();
  const XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
  return XMMatrixLookAtLH(eye, at, up);
}

XMMATRIX BuildProjection(const TeachingState& s, float aspect) {
  const float a = (aspect > 0.01f) ? aspect : (16.f / 9.f);
  if (s.proj == ProjOrtho) {
    const float h = s.camDistance;
    return XMMatrixOrthographicLH(h * a, h, s.nearZ, s.farZ);
  }
  return XMMatrixPerspectiveFovLH(XMConvertToRadians(s.fovDeg), a, s.nearZ, s.farZ);
}
