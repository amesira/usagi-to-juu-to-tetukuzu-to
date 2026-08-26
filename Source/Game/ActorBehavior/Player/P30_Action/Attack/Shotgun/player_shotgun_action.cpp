//===================================================
// File  ：_/P30_Action/Attack/Shotgun/player_shotgun_action.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・ショットガン攻撃アクション
//===================================================
#include "player_shotgun_action.h"

bool PlayerShotgunAction::CanStart(const PlayerContext& context, const PlayerInput& input)
{
    return false;
}

void PlayerShotgunAction::Start(PlayerContext& context, const PlayerInput& input)
{
}

void PlayerShotgunAction::Update(PlayerContext& context, const PlayerInput& input, float deltaTime)
{
}

void PlayerShotgunAction::Finish(PlayerContext& context, const PlayerInput& input)
{
}
