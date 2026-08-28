//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_firing.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃の発射処理
//---------------------------------------------------
#pragma once

struct PlayerDualPistolsContext;

class PlayerDualPistolsFiring {
public:
    struct FireRequest {
    };

    void Initialize(PlayerDualPistolsContext& context);
    void Fire(PlayerDualPistolsContext& context, const FireRequest& request);
};
