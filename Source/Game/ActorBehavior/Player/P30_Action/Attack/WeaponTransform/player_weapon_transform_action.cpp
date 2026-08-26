//===================================================
// File  ：_/P30_Action/Attack/WeaponTransform/player_weapon_transform_action.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・武器の変形アクション
//===================================================
#include "player_weapon_transform_action.h"

bool PlayerWeaponTransformAction::CanStart(const PlayerContext& context, const PlayerInput& input)
{
    return false;
}

void PlayerWeaponTransformAction::Start(PlayerContext& context, const PlayerInput& input)
{
}

void PlayerWeaponTransformAction::Update(PlayerContext& context, const PlayerInput& input, float deltaTime)
{
}

void PlayerWeaponTransformAction::Finish(PlayerContext& context, const PlayerInput& input)
{
}
