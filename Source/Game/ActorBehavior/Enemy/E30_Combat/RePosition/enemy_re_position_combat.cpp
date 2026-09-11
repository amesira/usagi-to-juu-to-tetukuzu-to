#include "enemy_re_position_combat.h"

#include <algorithm>
#include <cmath>
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/Move/enemy_path_follower.h"

void EnemyRePositionCombat::SetSettings(const EnemyRePositionSettings& settings)
{
    m_settings = settings;
    m_settings.arrivalDistance = (std::max)(0.01f, settings.arrivalDistance);
    m_settings.moveSpeedMultiplier = (std::max)(0.0f, settings.moveSpeedMultiplier);
    m_settings.timeout = (std::max)(0.01f, settings.timeout);
    m_settings.restartCooldown = (std::max)(0.0f, settings.restartCooldown);
}

void EnemyRePositionCombat::RequestPosition(const DirectX::XMFLOAT3& destination)
{
    m_destination = destination;
    m_requested = std::isfinite(destination.x) && std::isfinite(destination.y) && std::isfinite(destination.z);
    m_reachedDestination = false;
}

bool EnemyRePositionCombat::CanStart(const EnemyContext& context) const
{
    return m_requested && m_restartCooldown <= 0.0f && CanContinue(context);
}

bool EnemyRePositionCombat::CanContinue(const EnemyContext& context) const
{
    return context.runtimeState.hasCombatTarget && context.transform
        && context.pathFollower && context.locomotionController;
}

void EnemyRePositionCombat::UpdateBackground(EnemyContext&, float deltaTime)
{
    m_restartCooldown = (std::max)(0.0f, m_restartCooldown - (std::max)(0.0f, deltaTime));
}

void EnemyRePositionCombat::Start(EnemyContext& context)
{
    ReleaseResources();
    m_requested = false;
    m_reachedDestination = false;
    m_elapsedTime = 0.0f;
    m_navigation.Start(context, m_destination);
}

EnemyCombatStatus EnemyRePositionCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanContinue(context)) return EnemyCombatStatus::Failure;
    m_elapsedTime += (std::max)(0.0f, deltaTime);
    const auto status = m_navigation.Update(context, m_settings.arrivalDistance);
    m_reachedDestination = status == EnemyCombatStatus::Success;
    if (status != EnemyCombatStatus::Running) return status;
    if (m_elapsedTime >= m_settings.timeout || !UpdateLocomotion(context)) return EnemyCombatStatus::Failure;
    return EnemyCombatStatus::Running;
}

void EnemyRePositionCombat::Finish(EnemyContext&)
{
    ReleaseResources();
    m_requested = false;
    m_restartCooldown = m_settings.restartCooldown;
}

void EnemyRePositionCombat::Cancel(EnemyContext&)
{
    ReleaseResources();
    m_requested = false;
    m_reachedDestination = false;
    m_elapsedTime = 0.0f;
    m_restartCooldown = m_settings.restartCooldown;
}

bool EnemyRePositionCombat::UpdateLocomotion(const EnemyContext& context)
{
    if (m_controller != context.locomotionController) {
        if (m_controller) m_controller->RemoveRequest(m_requestHandle);
        m_requestHandle = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
    }
    m_controller = context.locomotionController;
    if (!m_controller || !context.pathFollower) return false;
    EnemyLocomotionController::LocomotionRequest request;
    request.priority = GetPriority();
    request.movementMode = EnemyMovementMode::ControlVelocity;
    request.moveSpeedMultiplier = m_settings.moveSpeedMultiplier;
    request.moveDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    request.moveDirection.fixedDirection = context.pathFollower->GetMoveDirection();
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    request.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    if (!m_controller->UpdateRequest(m_requestHandle, request)) m_requestHandle = m_controller->AddRequest(request);
    return m_requestHandle != EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

void EnemyRePositionCombat::ReleaseResources()
{
    if (m_controller) m_controller->RemoveRequest(m_requestHandle);
    m_controller = nullptr;
    m_requestHandle = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
    m_navigation.Cancel();
}
