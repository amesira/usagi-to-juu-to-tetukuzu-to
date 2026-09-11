#include "enemy_wait_combat.h"

#include <algorithm>
#include <cmath>
#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"

void EnemyWaitCombat::RequestWait(float duration, float minDistance, float maxDistance)
{
    m_waitDuration = (std::max)(0.0f, duration);
    m_minDistance = (std::max)(0.0f, minDistance);
    m_maxDistance = (std::max)(m_minDistance, maxDistance);
    m_remainingTime = m_waitDuration;
    m_requested = true;
    m_completed = false;
}

bool EnemyWaitCombat::CanStart(const EnemyContext& context) const
{
    return m_requested && CanContinue(context);
}

bool EnemyWaitCombat::CanContinue(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform || !context.locomotionController) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    const float distance = std::hypot(target.x - position.x, target.z - position.z);
    return distance >= m_minDistance && distance <= m_maxDistance;
}

void EnemyWaitCombat::Start(EnemyContext& context)
{
    ReleaseLocomotion();
    m_requested = false;
    m_completed = false;
    m_remainingTime = m_waitDuration;
    UpdateLocomotion(context);
}

EnemyCombatStatus EnemyWaitCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanContinue(context) || !UpdateLocomotion(context)) return EnemyCombatStatus::Failure;
    m_remainingTime = (std::max)(0.0f, m_remainingTime - (std::max)(0.0f, deltaTime));
    m_completed = m_remainingTime <= 0.0f;
    return m_completed ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
}

void EnemyWaitCombat::Finish(EnemyContext&)
{
    // Successの完了状態は次のRequestWait/Cancelまで保持する。
    ReleaseLocomotion();
    m_requested = false;
}

void EnemyWaitCombat::Cancel(EnemyContext&)
{
    ReleaseLocomotion();
    m_requested = false;
    m_completed = false;
    m_remainingTime = 0.0f;
}

bool EnemyWaitCombat::UpdateLocomotion(const EnemyContext& context)
{
    if (m_controller != context.locomotionController) ReleaseLocomotion();
    m_controller = context.locomotionController;
    if (!m_controller) return false;

    EnemyLocomotionController::LocomotionRequest request;
    request.priority = GetPriority();
    request.movementMode = EnemyMovementMode::StopHorizontal;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    request.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    if (!m_controller->UpdateRequest(m_requestHandle, request)) m_requestHandle = m_controller->AddRequest(request);
    return m_requestHandle != EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

void EnemyWaitCombat::ReleaseLocomotion()
{
    if (m_controller) m_controller->RemoveRequest(m_requestHandle);
    m_controller = nullptr;
    m_requestHandle = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}
