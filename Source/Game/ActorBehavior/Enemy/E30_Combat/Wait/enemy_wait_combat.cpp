//===================================================
// File  ：_/E30_Combat/Wait/enemy_wait_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_wait_combat.h"
#include <algorithm>
#include <cmath>

#include "Engine/Core/game_object.h"

#include "Engine/Component/transform_component.h"

#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_tree.h"

bool EnemyWaitCombat::CanStart(const EnemyContext& context) const
{
    return CanContinue(context);
}

bool EnemyWaitCombat::CanContinue(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform || !context.locomotionController) {
        return false;
    }
    return context.runtimeState.isInAttackRange && context.owner && context.owner->GetOwner()
        && context.combatTree && context.aiWorld && context.aiWorld->GetEnable()
        && context.aiWorld->IsInitialized();
}

void EnemyWaitCombat::Start(EnemyContext& context)
{
    m_completed = false;

    // LocomotionControllerへの要求を更新
    ReleaseLocomotion();
    UpdateLocomotion(context);
}

EnemyCombatStatus EnemyWaitCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanContinue(context) || !UpdateLocomotion(context)) {
        return EnemyCombatStatus::Failure;
    }

    // クールダウン中は停止して待つ。攻撃枠は予約しない
    if (!context.combatTree->IsAttackReady()) return EnemyCombatStatus::Running;
    const int enemyId = context.owner->GetOwner()->GetID();

    // 期限切れや外部取消後も再要求する。重複登録はCoordinatorが防ぐ
    context.aiWorld->RequestAttack({ enemyId });
    m_completed = context.aiWorld->CanAttack(enemyId);
    return m_completed ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
}

void EnemyWaitCombat::Finish(EnemyContext& context)
{
    if (!m_completed) {
        Cancel(context);
        return;
    }
    // 正常終了では予約を保持する。次のAttack::Startが消費する。
    ReleaseLocomotion();
}

void EnemyWaitCombat::Cancel(EnemyContext& context)
{
    ReleaseLocomotion();
    m_completed = false;
    if (context.aiWorld && context.owner && context.owner->GetOwner()) {
        context.aiWorld->CancelAttackRequest(context.owner->GetOwner()->GetID());
    }
}

#pragma region Locomotion
/// @brief LocomotionControllerへの要求を更新
bool EnemyWaitCombat::UpdateLocomotion(const EnemyContext& context)
{
    if (m_controller != context.locomotionController) ReleaseLocomotion();
    m_controller = context.locomotionController;
    if (!m_controller) return false;

    // combatTarget（プレイヤー）を見続けて待機
    EnemyLocomotionController::LocomotionRequest request;
    request.priority = GetPriority();
    request.movementMode = EnemyMovementMode::StopHorizontal;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    request.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    if (!m_controller->UpdateRequest(m_requestHandle, request)) {
        m_requestHandle = m_controller->AddRequest(request);
    }

    return m_requestHandle != EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

/// @brief LocomotionControllerへの要求を破棄
void EnemyWaitCombat::ReleaseLocomotion()
{
    if (m_controller) {
        m_controller->RemoveRequest(m_requestHandle);
    }
    m_controller = nullptr;
    m_requestHandle = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

#pragma endregion
