//===================================================
// File  ：_/E30_Combat/Approach/enemy_approach_combat.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_approach_combat.h"

#include <cmath>
#include <cstddef>
#include <utility>

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/Move/enemy_path_follower.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Engine/Component/transform_component.h"

bool EnemyApproachCombat::CanStart(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    return std::hypot(target.x - position.x, target.z - position.z) > m_attackDistance;
}

bool EnemyApproachCombat::CanContinue(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    return std::hypot(target.x - position.x, target.z - position.z) > m_attackDistance;
}

void EnemyApproachCombat::Start(EnemyContext& context)
{
    if (context.pathFollower) context.pathFollower->ClearPath();
    m_repathTimer = 0.0f;

    EnemyLocomotionController::LocomotionRequest request;
    request.priority = 10;
    request.movementMode = EnemyMovementMode::ControlVelocity;
    request.moveDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    m_locomotionRequestHandle = context.locomotionController
        ? context.locomotionController->AddRequest(request)
        : EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

EnemyCombatStatus EnemyApproachCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!context.aiWorld || !context.aiWorld->IsInitialized()) return EnemyCombatStatus::Failure;

    m_repathTimer -= deltaTime;
    if (m_repathTimer <= 0.0f) {
        UpdatePath(context);
        m_repathTimer = m_repathInterval;
    }
    UpdateLocomotionRequest(context);
    return EnemyCombatStatus::Running;
}

void EnemyApproachCombat::Finish(EnemyContext& context)
{
    Cancel(context);
}

void EnemyApproachCombat::Cancel(EnemyContext& context)
{
    if (context.locomotionController) {
        context.locomotionController->RemoveRequest(m_locomotionRequestHandle);
    }
    m_locomotionRequestHandle = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
    if (context.pathFollower) context.pathFollower->ClearPath();
}

void EnemyApproachCombat::UpdatePath(EnemyContext& context)
{
    if (!context.transform || !context.aiWorld || !context.pathFollower) return;

    auto result = context.aiWorld->FindPath(
        context.transform->GetPosition(),
        context.runtimeState.combatTargetPosition,
        context.aiAgentSettings().navigationAgent);

    context.pathFollower->ClearPath();
    if (result.status != EnemyAiWorld::PathQueryStatus::Success) return;

    // 現在のFindPathは [start, 始点セル中心, ... , goal] を返す。
    const size_t firstWaypoint = result.path.waypoints.size() > 2 ? 2 : 0;
    context.pathFollower->SetPath(std::move(result.path), firstWaypoint);
}

void EnemyApproachCombat::UpdateLocomotionRequest(EnemyContext& context)
{
    if (!context.transform || !context.locomotionController || !context.pathFollower) return;

    context.pathFollower->Update(context.transform->GetPosition());

    EnemyLocomotionController::LocomotionRequest request;
    request.priority = 10;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    request.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;

    const auto& moveDirection = context.pathFollower->GetMoveDirection();
    if (std::hypot(moveDirection.x, moveDirection.z) > 0.001f) {
        request.movementMode = EnemyMovementMode::ControlVelocity;
        request.moveDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
        request.moveDirection.fixedDirection = moveDirection;
    }
    else {
        request.movementMode = EnemyMovementMode::StopHorizontal;
    }

    context.locomotionController->UpdateRequest(m_locomotionRequestHandle, request);
}
