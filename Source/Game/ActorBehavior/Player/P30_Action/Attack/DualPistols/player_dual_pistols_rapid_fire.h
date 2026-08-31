//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_rapid_fire.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃の連射攻撃を制御するクラス
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"

struct PlayerDualPistolsContext;

class PlayerDualPistolsRapidFire {
private:
    bool m_isActive = false;
    float m_fireTimer = 0.0f;

    int m_locomotionRequestID = -1;
    PlayerLocomotionController::LocomotionRequest m_locomotionRequest;

public:
    void Initialize(PlayerDualPistolsContext& context);

    void Start(PlayerDualPistolsContext& context);
    void Update(PlayerDualPistolsContext& context, float deltaTime);
    void Stop(PlayerDualPistolsContext& context);
    void Reset(PlayerDualPistolsContext& context);

    bool IsActive() const { return m_isActive; }

private:
    void FireVolley(PlayerDualPistolsContext& context);
    void UpdateRapidFireAnimation(PlayerDualPistolsContext& context, float deltaTime);

};
