//===================================================
// File  ：_/DualPistols/player_dual_pistols_slash_burst.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_slash_burst.h"
#include "player_dual_pistols_context.h"

void PlayerDualPistolsSlashBurst::Initialize(PlayerDualPistolsContext& context)
{
}

void PlayerDualPistolsSlashBurst::Start(PlayerDualPistolsContext& context)
{
    m_isActive = true;
    m_isFinished = false;
}

void PlayerDualPistolsSlashBurst::Update(
    PlayerDualPistolsContext& context,
    float deltaTime)
{
}

void PlayerDualPistolsSlashBurst::Finish(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = true;
}

void PlayerDualPistolsSlashBurst::Cancel(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = true;
}

void PlayerDualPistolsSlashBurst::Reset(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = false;
}
