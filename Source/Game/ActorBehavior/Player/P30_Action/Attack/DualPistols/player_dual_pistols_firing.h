//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_firing.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃の発射処理
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

struct PlayerDualPistolsContext;

class PlayerDualPistolsFiring {
public:
    enum class PistolSide {
        Left,
        Right,
    };

    struct FireRequest {
        DirectX::XMFLOAT3 muzzlePosition{};
        DirectX::XMFLOAT3 fireDirection{};
        PistolSide pistolSide = PistolSide::Left;
    };

    void Initialize(PlayerDualPistolsContext& context);
    void Fire(PlayerDualPistolsContext& context, const FireRequest& request);
};
