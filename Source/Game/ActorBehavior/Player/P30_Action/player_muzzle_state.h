#pragma once

#include <DirectXMath.h>

/// @brief 1フレーム内で共有する銃口のワールド空間状態
struct PlayerMuzzleState {
    DirectX::XMFLOAT3 position{};
    DirectX::XMFLOAT4 rotation{};
    bool isValid = false;
};
