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

XMMATRIX BuildView(float distance, float pitchDeg, float yawDeg,
                   const float target[3]) {
  const float pitch = XMConvertToRadians(pitchDeg);
  const float yaw = XMConvertToRadians(yawDeg);
  const XMVECTOR at = XMVectorSet(target[0], target[1], target[2], 1.f);
  const XMVECTOR offset = XMVectorSet(
      distance * sinf(yaw) * cosf(pitch),
      distance * sinf(pitch),
      distance * cosf(yaw) * cosf(pitch),
      0.f);
  const XMVECTOR eye = XMVectorAdd(at, offset);
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

static float viewportAspect(float viewportW, float viewportH) {
  if (viewportH > 1.f) {
    return viewportW / viewportH;
  }
  return 16.f / 9.f;
}

bool projectWorldToScreen(const TeachingState& t, float wx, float wy, float wz,
                          float viewportW, float viewportH, float* sx, float* sy) {
  if (!sx || !sy || viewportW < 1.f || viewportH < 1.f) {
    return false;
  }
  const float aspect = t.aspectFollowViewport ? viewportAspect(viewportW, viewportH) : t.aspect;
  const XMMATRIX V = BuildView(t.camDistance, t.camPitchDeg, t.camYawDeg, t.camTarget);
  const XMMATRIX P = BuildProjection(t, aspect);
  const XMVECTOR clip = XMVector3TransformCoord(XMVectorSet(wx, wy, wz, 1.f), V * P);
  const float ndcX = XMVectorGetX(clip);
  const float ndcY = XMVectorGetY(clip);
  const float ndcZ = XMVectorGetZ(clip);
  if (ndcZ < 0.f || ndcZ > 1.f) {
    return false;
  }
  *sx = (ndcX * 0.5f + 0.5f) * viewportW;
  *sy = (1.f - (ndcY * 0.5f + 0.5f)) * viewportH;
  return true;
}

static float distPointToSegment2(float px, float py, float ax, float ay, float bx, float by) {
  const float abx = bx - ax;
  const float aby = by - ay;
  const float apx = px - ax;
  const float apy = py - ay;
  const float ab2 = abx * abx + aby * aby;
  float u = 0.f;
  if (ab2 > 1e-8f) {
    u = (apx * abx + apy * aby) / ab2;
    if (u < 0.f) {
      u = 0.f;
    }
    if (u > 1.f) {
      u = 1.f;
    }
  }
  const float dx = px - (ax + abx * u);
  const float dy = py - (ay + aby * u);
  return dx * dx + dy * dy;
}

int hitWorldAxisHandle(const TeachingState& t, float mx, float my,
                       float viewportW, float viewportH) {
  const float* pos = t.objects[0].trs.pos;
  const float len = worldAxisGizmoLength();
  float ox = 0.f;
  float oy = 0.f;
  if (!projectWorldToScreen(t, pos[0], pos[1], pos[2], viewportW, viewportH, &ox, &oy)) {
    return -1;
  }
  const float dirs[3][3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};
  int best = -1;
  float bestD2 = 12.f * 12.f;
  for (int i = 0; i < 3; ++i) {
    float ex = 0.f;
    float ey = 0.f;
    if (!projectWorldToScreen(t, pos[0] + dirs[i][0] * len, pos[1] + dirs[i][1] * len,
                              pos[2] + dirs[i][2] * len, viewportW, viewportH, &ex, &ey)) {
      continue;
    }
    const float d2 = distPointToSegment2(mx, my, ox, oy, ex, ey);
    if (d2 < bestD2) {
      bestD2 = d2;
      best = i;
    }
  }
  return best;
}

float axisTranslateFromDrag(const TeachingState& t, int axis, float dx, float dy,
                            float viewportW, float viewportH) {
  if (axis < 0 || axis > 2) {
    return 0.f;
  }
  const float* pos = t.objects[0].trs.pos;
  const float len = worldAxisGizmoLength();
  const float dirs[3][3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};
  float ox = 0.f;
  float oy = 0.f;
  float ex = 0.f;
  float ey = 0.f;
  if (!projectWorldToScreen(t, pos[0], pos[1], pos[2], viewportW, viewportH, &ox, &oy)) {
    return 0.f;
  }
  if (!projectWorldToScreen(t, pos[0] + dirs[axis][0] * len, pos[1] + dirs[axis][1] * len,
                            pos[2] + dirs[axis][2] * len, viewportW, viewportH, &ex, &ey)) {
    return 0.f;
  }
  const float sx = ex - ox;
  const float sy = ey - oy;
  const float s2 = sx * sx + sy * sy;
  if (s2 < 1e-6f) {
    return 0.f;
  }
  return ((dx * sx + dy * sy) / s2) * len;
}
