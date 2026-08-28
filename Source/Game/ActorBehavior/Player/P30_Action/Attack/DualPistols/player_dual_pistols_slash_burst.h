//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_slash_burst.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃による薙ぎ払い回転攻撃を制御するクラス
//---------------------------------------------------
#pragma once

struct PlayerDualPistolsContext;

class PlayerDualPistolsSlashBurst {
private:
    bool m_isActive = false;
    bool m_isFinished = false;

public:
    void Initialize(PlayerDualPistolsContext& context);

    void Start(PlayerDualPistolsContext& context);
    void Update(PlayerDualPistolsContext& context, float deltaTime);
    void Finish(PlayerDualPistolsContext& context);
    void Cancel(PlayerDualPistolsContext& context);
    void Reset(PlayerDualPistolsContext& context);

    bool IsActive() const { return m_isActive; }
    bool IsFinished() const { return m_isFinished; }
};
