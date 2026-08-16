#pragma once
#include "core/TeachingState.h"
#include <DirectXMath.h>

DirectX::XMMATRIX BuildWorld(const TransformTRS& trs);
DirectX::XMMATRIX BuildView(float distance, float pitchDeg, float yawDeg);
DirectX::XMMATRIX BuildProjection(const TeachingState& s, float aspect);
