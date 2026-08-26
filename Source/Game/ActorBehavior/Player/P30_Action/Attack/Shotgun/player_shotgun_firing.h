//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_firing.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの発射処理を制御するクラス
//---------------------------------------------------
#pragma once

#include "player_shotgun_aim.h"

struct PlayerShotgunContext;

class PlayerShotgunFiring {
public:
    struct FireRequest {
        PlayerShotgunAim::AimResult aimResult = {};
        float chargeRate = 0.0f;
    };

    struct FireResult {
        int projectileCount = 0;
    };

    /// @brief 指定された照準情報とチャージ率で弾を発射する
    FireResult Fire(PlayerShotgunContext& context, const FireRequest& request);
};
