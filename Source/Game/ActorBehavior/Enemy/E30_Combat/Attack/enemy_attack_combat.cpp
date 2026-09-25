//===================================================
// File  ：_/E_30_Combat/Attack/enemy_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_attack_combat.h"
#include <algorithm>
#include <cmath>

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Wait/enemy_wait_combat.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Engine/Core/game_object.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"

void EnemyAttackCombat::Initialize(EnemyContext& context)
{
    m_context.combat = this;
    m_context.owner = context.owner->GetOwner();
    m_context.scene = context.scene;
    m_context.transform = context.transform;
    m_context.rigidbody = context.rigidbody;
}

bool EnemyAttackCombat::CanStart(const EnemyContext& context) const
{
    if (!CanContinue(context) || m_context.runtimeState.cooldownRemaining > 0.0f) return false;

    // 射程内で、かつCoordinatorが攻撃可能と判断した場合のみ開始する
    return context.runtimeState.isInAttackRange
        && context.aiWorld->CanAttack(context.owner->GetOwner()->GetID());
}

bool EnemyAttackCombat::CanContinue(const EnemyContext& context) const
{
    // 開始後の射程離脱では中断しない。Stun/DeadはTreeがCancelする。
    return context.runtimeState.hasCombatTarget && context.transform
        && context.owner && context.owner->GetOwner()
        && context.aiWorld && context.aiWorld->GetEnable() && context.aiWorld->IsInitialized()
        && (!m_hasAttackSlot || context.aiWorld->GetAttackCoordinatorSystem().IsAttacking(
            context.owner->GetOwner()->GetID()));
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

    // 攻撃枠を消費して攻撃開始
    m_hasAttackSlot = CanStart(context) && context.aiWorld->ConsumeAttackRequest(
        context.owner->GetOwner()->GetID());
    if (!m_hasAttackSlot) return;

    auto& state = m_context.runtimeState;
    state.aimPosition = context.runtimeState.combatTargetPosition;

    m_context.runtimeState.phase = EnemyAttackPhase::Idle;
    ChangePhase(EnemyAttackPhase::Windup);
}

EnemyCombatStatus EnemyAttackCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!m_hasAttackSlot || !CanContinue(context)) return EnemyCombatStatus::Failure;
    const float dt = (std::max)(0.0f, deltaTime);

    auto& state = m_context.runtimeState;
    state.phaseElapsed += dt;

    bool enteredPhase = m_enteredPhase;
    m_enteredPhase = false;

    switch (state.phase) {
    case EnemyAttackPhase::Windup: {
        if (enteredPhase) {
            BeginWindup(context);
        }
        UpdateWindup(context, dt);
        state.aimPosition = context.runtimeState.combatTargetPosition;
        if (state.phaseElapsed >= settings().windupDuration) {
            EndWindup(context);
            ChangePhase(EnemyAttackPhase::Active);
        }
        return EnemyCombatStatus::Running;
    }
    case EnemyAttackPhase::Active: {
        if (enteredPhase) {
            m_attackEntered = true;
            BeginAttack(context);
        }

        const auto status = UpdateAttack(context, dt);

        if (status == EnemyCombatStatus::Failure) return status;
        if (status == EnemyCombatStatus::Success) {
            CloseAttack(context);
            ChangePhase(EnemyAttackPhase::Recovery);
        }
        return EnemyCombatStatus::Running;
    }
    case EnemyAttackPhase::Recovery: {
        // 後隙は待機するだけ
        return state.phaseElapsed >= settings().recoveryDuration
            ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
    }
    default: return EnemyCombatStatus::Failure;
    }
}

void EnemyAttackCombat::ChangePhase(EnemyAttackPhase newPhase)
{
    if (m_context.runtimeState.phase == newPhase) return;
    m_context.runtimeState.phase = newPhase;
    m_context.runtimeState.phaseElapsed = 0.0f;
    m_enteredPhase = true;
}

void EnemyAttackCombat::Finish(EnemyContext& context) { Cancel(context); }

void EnemyAttackCombat::Cancel(EnemyContext& context)
{
    const bool wasActive = m_context.runtimeState.phase != EnemyAttackPhase::Idle;
    Release(context);
    if (wasActive) m_context.runtimeState.cooldownRemaining = settings().restartCooldown;
}

/// @brief 攻撃対象との距離が射程内かどうかを判定する
bool EnemyAttackCombat::IsInAttackRange(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    const float distance = std::hypot(target.x - position.x, target.z - position.z);
    return distance >= settings().minDistance && distance <= settings().maxDistance;
}

#pragma region 攻撃解放
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
    if (m_hasAttackSlot && context.aiWorld && context.owner && context.owner->GetOwner()) {
        context.aiWorld->FinishAttack(context.owner->GetOwner()->GetID());
    }
    m_hasAttackSlot = false;
}
#pragma endregion
