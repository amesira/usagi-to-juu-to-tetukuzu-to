//===================================================
// File  ：_/E_30_Combat/Attack/enemy_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_attack_combat.h"
#include <algorithm>
#include <cmath>
#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Wait/enemy_wait_combat.h"

bool EnemyAttackCombat::CanStart(const EnemyContext& context) const
{
    if (!CanContinue(context) || m_context.runtimeState.cooldownRemaining > 0.0f) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    const float distance = std::hypot(target.x - position.x, target.z - position.z);
    return distance >= settings().minDistance && distance <= settings().maxDistance;
}

bool EnemyAttackCombat::CanContinue(const EnemyContext& context) const
{
    // 開始後の射程離脱では中断しない。Stun/DeadはTreeがCancelする。
    return context.runtimeState.hasCombatTarget && context.transform;
}

bool EnemyAttackCombat::IsInterruptible(const EnemyContext&) const
{
    const auto phase = m_context.runtimeState.phase;
    return phase == EnemyAttackPhase::Idle
        || (phase == EnemyAttackPhase::Windup && settings().interruptibleWindup)
        || (phase == EnemyAttackPhase::Recovery && settings().interruptibleRecovery);
}

void EnemyAttackCombat::UpdateBackground(EnemyContext&, float deltaTime)
{
    auto& cooldown = m_context.runtimeState.cooldownRemaining;
    cooldown = (std::max)(0.0f, cooldown - (std::max)(0.0f, deltaTime));
}

void EnemyAttackCombat::Start(EnemyContext& context)
{
    Release(context);
    auto& state = m_context.runtimeState;
    state.phase = EnemyAttackPhase::Windup;
    state.phaseElapsed = 0.0f;
    state.aimPosition = context.runtimeState.combatTargetPosition;
    BeginWindup(context);
}

EnemyCombatStatus EnemyAttackCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanContinue(context)) return EnemyCombatStatus::Failure;
    const float dt = (std::max)(0.0f, deltaTime);
    auto& state = m_context.runtimeState;
    state.phaseElapsed += dt;
    // 段階の切り替えは1更新に1回。新しい段階の時間は次の更新から計測する。
    switch (state.phase) {
    case EnemyAttackPhase::Windup:
        state.aimPosition = context.runtimeState.combatTargetPosition;
        if (state.phaseElapsed >= settings().windupDuration) {
            EndWindup(context);
            state.phase = EnemyAttackPhase::Active;
            state.phaseElapsed = 0.0f;
            m_attackEntered = true;
            BeginAttack(context);
        }
        return EnemyCombatStatus::Running;
    case EnemyAttackPhase::Active: {
        const auto status = UpdateAttack(context, dt);
        if (status == EnemyCombatStatus::Failure) return status;
        if (status == EnemyCombatStatus::Success) {
            CloseAttack(context);
            state.phase = EnemyAttackPhase::Recovery;
            state.phaseElapsed = 0.0f;
        }
        return EnemyCombatStatus::Running;
    }
    case EnemyAttackPhase::Recovery:
        return state.phaseElapsed >= settings().recoveryDuration
            ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
    default:
        return EnemyCombatStatus::Failure;
    }
}

void EnemyAttackCombat::Finish(EnemyContext& context) { Cancel(context); }

void EnemyAttackCombat::Cancel(EnemyContext& context)
{
    const bool wasActive = m_context.runtimeState.phase != EnemyAttackPhase::Idle;
    Release(context);
    if (wasActive) m_context.runtimeState.cooldownRemaining = settings().restartCooldown;
}

bool EnemyAttackCombat::IsInAttackRange(EnemyContext& context) const
{
    if (!context.transform) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    const float distance = std::hypot(target.x - position.x, target.z - position.z);
    return distance >= settings().minDistance && distance <= settings().maxDistance;
}

void EnemyAttackCombat::CloseAttack(EnemyContext& context)
{
    if (!m_attackEntered) return;
    m_attackEntered = false;
    EndAttack(context);
}

void EnemyAttackCombat::Release(EnemyContext& context)
{
    CloseAttack(context);
    if (m_context.runtimeState.phase == EnemyAttackPhase::Windup) EndWindup(context);
    m_context.runtimeState.phase = EnemyAttackPhase::Idle;
    m_context.runtimeState.phaseElapsed = 0.0f;
}
