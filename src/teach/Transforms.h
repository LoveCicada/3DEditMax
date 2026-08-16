#pragma once
#include "core/TeachingState.h"
#include <DirectXMath.h>
#include <cmath>

DirectX::XMMATRIX BuildWorld(const TransformTRS& trs);
DirectX::XMMATRIX BuildView(float distance, float pitchDeg, float yawDeg,
                            const float target[3]);
DirectX::XMMATRIX BuildProjection(const TeachingState& s, float aspect);

struct TrackResult {
  DirectX::XMFLOAT4 model;
  DirectX::XMFLOAT4 world;
  DirectX::XMFLOAT4 view;
  DirectX::XMFLOAT4 clip;
  DirectX::XMFLOAT4 ndc;
};

inline TrackResult TrackPoint(DirectX::XMFLOAT3 pModel,
                              DirectX::FXMMATRIX w,
                              DirectX::FXMMATRIX v,
                              DirectX::FXMMATRIX p) {
  using namespace DirectX;
  TrackResult r;
  const XMVECTOR m = XMVectorSet(pModel.x, pModel.y, pModel.z, 1.f);
  const XMVECTOR wo = XMVector4Transform(m, w);
  const XMVECTOR vi = XMVector4Transform(wo, v);
  const XMVECTOR cl = XMVector4Transform(vi, p);
  XMStoreFloat4(&r.model, m);
  XMStoreFloat4(&r.world, wo);
  XMStoreFloat4(&r.view, vi);
  XMStoreFloat4(&r.clip, cl);
  const float cw = XMVectorGetW(cl);
  XMVECTOR nd = cl;
  if (fabsf(cw) > 1e-8f) {
    nd = XMVectorScale(cl, 1.f / cw);
  }
  XMStoreFloat4(&r.ndc, nd);
  return r;
}
