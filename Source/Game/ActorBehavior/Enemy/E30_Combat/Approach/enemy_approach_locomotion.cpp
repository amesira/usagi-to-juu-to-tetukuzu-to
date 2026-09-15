//===================================================
// File  ：_/E30_Combat/Approach/enemy_approach_locomotion.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_approach_locomotion.h"
#include "enemy_approach_context.h"

#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/Move/enemy_path_follower.h"

#include <cmath>

bool EnemyApproachLocomotion::Update(EnemyApproachContext& context)
{
    if (m_controller != context.locomotionController) Cancel();
    m_controller = context.locomotionController;

    if (!m_controller || !context.pathFollower || !context.enemyRuntimeState) {
        return false;
    }

    // LocomotionRequestを構築してLocomotionControllerに渡す
    EnemyLocomotionController::LocomotionRequest request;
    request.priority = 10;
    request.moveSpeedMultiplier = context.settings().moveSpeedMultiplier;
    const auto direction = context.pathFollower->GetMoveDirection();
    request.movementMode = EnemyMovementMode::ControlVelocity;
    request.moveDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    request.moveDirection.fixedDirection = direction;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    request.rotateDirection = request.moveDirection;

    if (!m_controller->UpdateRequest(m_requestHandle, request)) {
        m_requestHandle = m_controller->AddRequest(request);
    }

    return m_requestHandle != EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

void EnemyApproachLocomotion::Cancel()
{
    if (m_controller) {
        m_controller->RemoveRequest(m_requestHandle);
    }

    m_controller = nullptr;
    m_requestHandle = -1;
}