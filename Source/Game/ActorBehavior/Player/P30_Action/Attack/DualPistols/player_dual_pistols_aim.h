//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_aim.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃の照準処理
//---------------------------------------------------
#pragma once

struct PlayerDualPistolsContext;

class PlayerDualPistolsAim {
public:
    void Initialize(PlayerDualPistolsContext& context);
    void EnterAim(PlayerDualPistolsContext& context);
    void UpdateAim(PlayerDualPistolsContext& context, float deltaTime);
    void ExitAim(PlayerDualPistolsContext& context);
};
