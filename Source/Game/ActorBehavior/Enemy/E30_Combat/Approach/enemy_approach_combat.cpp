//===================================================
// File  ：_/E30_Combat/Approach/enemy_approach_combat.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_approach_combat.h"
#include "Engine/Component/transform_component.h"
#include <cmath>

#include "Engine/Core/game_object.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"

void EnemyApproachCombat::Initialize(const EnemyContext& context, const EnemyApproachSettingsAsset* settings)
{
    m_context.owner = this;
    m_context.gameObjectID = context.owner ? context.owner->GetOwner()->GetID() : -1;
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
    m_context.combatTree = context.combatTree;
    m_context.aiAgentSettingsAsset = context.aiAgentSettingsAsset;
}

bool EnemyApproachCombat::CanStart(const EnemyContext& context) const
{
    if (!CanContinue(context) || context.runtimeState.isInAttackRange || m_restartCooldown > 0.0f) {
        return false;
    }

    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;

    float distanceToTarget = std::hypot(target.x - position.x, target.z - position.z);
    const bool hasAttack = m_context.combatTree && m_context.combatTree->GetAttackCombat();
    float restartMargin = !hasAttack && m_context.runtimeState.hasReachedDestination
        ? m_context.settings().restartDistanceMargin : 0.0f;
    return distanceToTarget > GetStopDistance() + restartMargin;
}

bool EnemyApproachCombat::CanContinue(const EnemyContext& context) const
{
    // ターゲットがいない、Transformがない場合は継続不可
    return context.runtimeState.hasCombatTarget && context.transform;
}

float EnemyApproachCombat::GetStopDistance() const
{
    const auto* attack = m_context.combatTree ? m_context.combatTree->GetAttackCombat() : nullptr;
    return attack ? attack->GetMaxAttackDistance() : m_context.settings().stopDistance;
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
    default: break;
    }

    // Locomotion由来の失敗も、最終的なstatusでクールダウンを設定する。
    if (status == EnemyCombatStatus::Failure) {
        m_restartCooldown = m_context.settings().pathRetryInterval;
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
