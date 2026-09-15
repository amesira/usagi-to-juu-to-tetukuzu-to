//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_firing.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの発射処理を制御するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
struct PlayerShotgunContext;

class PlayerShotgunFiring {
public:
    struct FireRequest {
        DirectX::XMFLOAT3 muzzlePosition{};
        DirectX::XMFLOAT3 fireDirection{};
        float chargeRate = 0.0f;
    };

    /// @brief 指定された照準情報とチャージ率で弾を発射する
    void Fire(PlayerShotgunContext& context, const FireRequest& request);
};
