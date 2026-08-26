//---------------------------------------------------
// File  ：_/P30_Action/Attack/WeaponTransform/player_weapon_transform_action.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・武器の変形アクション
//---------------------------------------------------
#pragma once

#include "Game/ActorBehavior/Player/P30_Action/player_action_base.h"

class PlayerWeaponTransformAction : public PlayerActionBase {
public:
    PlayerWeaponTransformAction() : PlayerActionBase("WeaponTransform", ActionCategory::Attack, 10, true) {}
    bool CanStart(const PlayerContext& context, const PlayerInput& input) override;

    void Start(PlayerContext& context, const PlayerInput& input) override;
    void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) override;
    void Finish(PlayerContext& context, const PlayerInput& input) override;
};
