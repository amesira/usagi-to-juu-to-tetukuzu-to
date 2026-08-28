//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_rapid_fire.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃の連射攻撃を制御するクラス
//---------------------------------------------------
#pragma once

struct PlayerDualPistolsContext;

class PlayerDualPistolsRapidFire {
private:
    bool m_isActive = false;

public:
    void Initialize(PlayerDualPistolsContext& context);

    void Start(PlayerDualPistolsContext& context);
    void Update(PlayerDualPistolsContext& context, float deltaTime);
    void Stop(PlayerDualPistolsContext& context);
    void Reset(PlayerDualPistolsContext& context);

    bool IsActive() const { return m_isActive; }
};
