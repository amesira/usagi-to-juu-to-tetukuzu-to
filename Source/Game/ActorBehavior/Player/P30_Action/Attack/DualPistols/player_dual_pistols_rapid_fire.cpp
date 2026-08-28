//===================================================
// File  ：_/DualPistols/player_dual_pistols_rapid_fire.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_rapid_fire.h"
#include "player_dual_pistols_context.h"

void PlayerDualPistolsRapidFire::Initialize(PlayerDualPistolsContext& context)
{
}

void PlayerDualPistolsRapidFire::Start(PlayerDualPistolsContext& context)
{
    m_isActive = true;
}

void PlayerDualPistolsRapidFire::Update(
    PlayerDualPistolsContext& context,
    float deltaTime)
{
}

void PlayerDualPistolsRapidFire::Stop(PlayerDualPistolsContext& context)
{
    m_isActive = false;
}

void PlayerDualPistolsRapidFire::Reset(PlayerDualPistolsContext& context)
{
    m_isActive = false;
}
