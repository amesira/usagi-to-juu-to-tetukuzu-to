//---------------------------------------------------
// File  ：_/P30_Action/Attack/DualPistols/player_dual_pistols_action.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・二丁拳銃の攻撃アクション
//---------------------------------------------------
#pragma once

#include "Game/ActorBehavior/Player/P30_Action/player_action_base.h"

class PlayerDualPistolsAction : public PlayerActionBase {
public:
    PlayerDualPistolsAction() : PlayerActionBase("DualPistols", ActionCategory::Attack, 10, true) {}
    bool CanStart(const PlayerContext& context, const PlayerInput& input) override;

    void Start(PlayerContext& context, const PlayerInput& input) override;
    void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) override;
    void Finish(PlayerContext& context, const PlayerInput& input) override;
};
