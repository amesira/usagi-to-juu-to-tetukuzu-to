//===================================================
// File  ：_/E30_Combat/Approach/enemy_approach_combat.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_approach_combat.h"
#include "Engine/Component/transform_component.h"
#include <cmath>

void EnemyApproachCombat::Initialize(const EnemyContext& context, const EnemyApproachSettingsAsset* settings)
{
    m_context.owner = this;
    m_context.settingsAsset = settings;
    m_restartCooldown = 0.0f;

    if (m_context.pathFollower != context.pathFollower) {
        m_context.navigation.Cancel(m_context);
    }

    m_context.enemyTransform = context.transform;
    m_context.enemyRuntimeState = &context.runtimeState;
    m_context.aiWorld = context.aiWorld;
    m_context.pathFollower = context.pathFollower;
    m_context.locomotionController = context.locomotionController;
    m_context.aiAgentSettingsAsset = context.aiAgentSettingsAsset;
}

bool EnemyApproachCombat::CanStart(const EnemyContext& context) const
{
    if (!CanContinue(context) || m_restartCooldown > 0.0f) {
        return false;
    }

    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;

    float distanceToTarget = std::hypot(target.x - position.x, target.z - position.z);
    float restartMargin = m_context.runtimeState.hasReachedDestination ? m_context.settings().restartDistanceMargin : 0.0f;
    return distanceToTarget > GetStopDistance() + restartMargin;
}

bool EnemyApproachCombat::CanContinue(const EnemyContext& context) const
{
    // ターゲットがいない、Transformがない場合は継続不可
    return context.runtimeState.hasCombatTarget && context.transform;
}

void EnemyApproachCombat::UpdateBackground(EnemyContext&, float deltaTime)
{
    m_restartCooldown -= deltaTime;
    if (m_restartCooldown < 0.0f) m_restartCooldown = 0.0f;
}

void EnemyApproachCombat::Start(EnemyContext& context)
{
    m_context.locomotion.Cancel();
    m_context.navigation.Start(m_context);
}

EnemyCombatStatus EnemyApproachCombat::Update(EnemyContext& context, float deltaTime)
{
    // Navigationの更新を行う
    auto status = m_context.navigation.Update(m_context, deltaTime);

    switch (status) {
    case EnemyCombatStatus::Running: {
        if (!m_context.locomotion.Update(m_context)) {
            status = EnemyCombatStatus::Failure;
        }
        break;
    }
    case EnemyCombatStatus::Failure: {
        m_restartCooldown = m_context.settings().pathRetryInterval;
        break;
    }
    default: break;
    }

    return status;
}

void EnemyApproachCombat::Finish(EnemyContext& context) 
{ 
    Cancel(context);
}

void EnemyApproachCombat::Cancel(EnemyContext&)
{
    m_context.locomotion.Cancel();
    m_context.navigation.Cancel(m_context);
}
