//===================================================
// File  ：_/E30_Combat/Approach/enemy_approach_combat.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_approach_combat.h"

#include <cmath>
#include <utility>

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"

#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"

#include "Engine/Component/transform_component.h"

bool EnemyApproachCombat::CanStart(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    return std::hypot(target.x - position.x, target.z - position.z) > m_attackDistance;
}

void EnemyApproachCombat::Start(EnemyContext& context)
{
    ClearPath();
    m_repathTimer = 0.0f;

    EnemyLocomotionController::LocomotionRequest request;
    request.priority = 10;
    request.moveSpeed = m_moveSpeed;
    request.moveDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    m_locomotionRequestHandle = context.locomotionController
        ? context.locomotionController->AddRequest(request)
        : EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

EnemyCombatStatus EnemyApproachCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanStart(context)) return EnemyCombatStatus::Success;
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
    ClearPath();
}

void EnemyApproachCombat::UpdatePath(EnemyContext& context)
{
    if (!context.transform || !context.aiWorld) return;

    static const EnemyAiAgentSettings::Data defaultSettings;
    const auto& settings = context.aiAgentSettingsAsset
        ? context.aiAgentSettingsAsset->GetData()
        : defaultSettings;
    auto result = context.aiWorld->FindPath(
        context.transform->GetPosition(),
        context.runtimeState.combatTargetPosition,
        settings.navigationAgent);

    ClearPath();
    if (result.status != EnemyAiWorld::PathQueryStatus::Success) return;
    m_path = std::move(result.path);
    // FindPathの先頭は実座標と始点セル中心。現在セル中心への引き返しを避ける。
    m_waypointIndex = m_path.waypoints.size() > 2 ? 2 : 0;
}

void EnemyApproachCombat::UpdateLocomotionRequest(EnemyContext& context)
{
    if (!context.transform || !context.locomotionController) return;

    const auto position = context.transform->GetPosition();
    while (m_waypointIndex < m_path.waypoints.size()) {
        const auto waypoint = m_path.waypoints[m_waypointIndex];
        const float dx = waypoint.x - position.x;
        const float dz = waypoint.z - position.z;
        if (std::hypot(dx, dz) <= m_reachDistance) {
            ++m_waypointIndex;
            continue;
        }

        EnemyLocomotionController::LocomotionRequest request;
        request.priority = 10;
        request.moveSpeed = m_moveSpeed;
        request.moveDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
        request.moveDirection.fixedDirection = { dx, 0.0f, dz };
        request.rotateDirection = request.moveDirection;
        context.locomotionController->UpdateRequest(m_locomotionRequestHandle, request);
        return;
    }

    EnemyLocomotionController::LocomotionRequest stopRequest;
    stopRequest.priority = 10;
    stopRequest.canMove = false;
    stopRequest.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    stopRequest.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    context.locomotionController->UpdateRequest(m_locomotionRequestHandle, stopRequest);
}

void EnemyApproachCombat::ClearPath()
{
    m_path.waypoints.clear();
    m_waypointIndex = 0;
}
