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

bool EnemyWaitCombat::CanStart(const EnemyContext& context) const
{
    return context.runtimeState.isInAttackRange;
}

bool EnemyWaitCombat::CanContinue(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform || !context.locomotionController) {
        return false;
    }


}

void EnemyWaitCombat::Start(EnemyContext& context)
{
    // 攻撃要求をEnemyAIWorldに送信
    if (auto* enemyAiWorld = Game::EnemyAIWorld()) {
        AttackCoordinatorSystem::AttackRequest request;
        request.enemyId = context.owner->GetOwner()->GetID();
        enemyAiWorld->RequestAttack(request);
    }

    // LocomotionControllerへの要求を更新
    ReleaseLocomotion();
    UpdateLocomotion(context);
}

EnemyCombatStatus EnemyWaitCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanContinue(context) || !UpdateLocomotion(context)) {
        return EnemyCombatStatus::Failure;
    }

    return EnemyCombatStatus::Running;
}

void EnemyWaitCombat::Finish(EnemyContext&)
{
    ReleaseLocomotion();
}

void EnemyWaitCombat::Cancel(EnemyContext&)
{
    ReleaseLocomotion();
}

#pragma region Locomotion
/// @brief LocomotionControllerへの要求を更新
bool EnemyWaitCombat::UpdateLocomotion(const EnemyContext& context)
{
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