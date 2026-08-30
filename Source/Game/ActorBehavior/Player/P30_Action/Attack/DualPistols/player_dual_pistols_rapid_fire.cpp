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
    m_fireTimer = 0.0f;

    context.aim.UpdateAim(context, 0.0f);
    FireVolley(context);
}

void PlayerDualPistolsRapidFire::Update(
    PlayerDualPistolsContext& context,
    float deltaTime)
{
    if (!m_isActive) return;

    context.aim.UpdateAim(context, deltaTime);

    const float fireInterval = context.settings().rapidFireInterval;
    if (fireInterval <= 0.0f) return;

    m_fireTimer += deltaTime;
    while (m_fireTimer >= fireInterval) {
        m_fireTimer -= fireInterval;
        FireVolley(context);
    }
}

void PlayerDualPistolsRapidFire::Stop(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_fireTimer = 0.0f;
}

void PlayerDualPistolsRapidFire::Reset(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_fireTimer = 0.0f;
}

void PlayerDualPistolsRapidFire::FireVolley(PlayerDualPistolsContext& context)
{
    const auto& aimResult = context.aim.GetAimResult();

    if (aimResult.hasLeftMuzzle) {
        PlayerDualPistolsFiring::FireRequest request;
        request.muzzlePosition = aimResult.leftMuzzlePosition;
        request.fireDirection = aimResult.leftFireDirection;
        request.pistolSide = PlayerDualPistolsFiring::PistolSide::Left;
        context.firing.Fire(context, request);
    }

    if (aimResult.hasRightMuzzle) {
        PlayerDualPistolsFiring::FireRequest request;
        request.muzzlePosition = aimResult.rightMuzzlePosition;
        request.fireDirection = aimResult.rightFireDirection;
        request.pistolSide = PlayerDualPistolsFiring::PistolSide::Right;
        context.firing.Fire(context, request);
    }
}
