//===================================================
// File  ：_/P30_Action/Attack/DualPistols/player_dual_pistols_action.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・二丁拳銃の攻撃アクション
//===================================================
#include "player_dual_pistols_action.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"

void PlayerDualPistolsAction::Initialize(
    const PlayerContext& context,
    PlayerDualPistolsReferences references,
    PlayerDualPistolsSettingsAsset* settingsAsset)
{
    m_context.owner = this;
    m_context.scene = context.scene;
    m_context.references = references;
    m_context.settingsAsset = settingsAsset;

    m_context.aim.Initialize(m_context);
    m_context.firing.Initialize(m_context);
    m_context.effects.Initialize(m_context);
    m_context.rapidFire.Initialize(m_context);
    m_context.slashBurst.Initialize(m_context);
}

bool PlayerDualPistolsAction::CanStart(const PlayerContext& context, const PlayerInput& input)
{
    return false;
}

void PlayerDualPistolsAction::Start(PlayerContext& context, const PlayerInput& input)
{
}

void PlayerDualPistolsAction::Update(PlayerContext& context, const PlayerInput& input, float deltaTime)
{
}

void PlayerDualPistolsAction::Finish(PlayerContext& context, const PlayerInput& input)
{
}
