#pragma once

#include <DirectXMath.h>

/// @brief エフェクトの位置、回転、スケーリングをまとめた軽量な値型
struct EffectTransform {
    DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT3 scaling{ 1.0f, 1.0f, 1.0f };
};
