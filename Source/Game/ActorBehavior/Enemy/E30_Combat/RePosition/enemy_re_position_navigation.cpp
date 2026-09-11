//===================================================
// File  ：_/RePosition/enemy_re_position_navigation.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_re_position_navigation.h"

#include <cmath>
#include <utility>
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/Move/enemy_path_follower.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"

void EnemyRePositionNavigation::Start(const EnemyContext& context, const DirectX::XMFLOAT3& destination)
{
    Cancel();
    m_destination = destination;
    m_pathFollower = context.pathFollower;
    if (m_pathFollower) m_pathFollower->ClearPath();
}

EnemyCombatStatus EnemyRePositionNavigation::Update(const EnemyContext& context, float arrivalDistance)
{
    if (!context.transform || !m_pathFollower || m_pathFollower != context.pathFollower
        || !context.aiWorld || !context.aiWorld->GetEnable() || !context.aiWorld->IsInitialized()) {
        return EnemyCombatStatus::Failure;
    }
    const auto position = context.transform->GetPosition();
    if (std::hypot(m_destination.x - position.x, m_destination.z - position.z) <= arrivalDistance) {
        return EnemyCombatStatus::Success;
    }
    if (!m_hasPath) {
        const int ownerID = context.owner && context.owner->GetOwner() ? context.owner->GetOwner()->GetID() : -1;
        auto result = context.aiWorld->FindPath(position, m_destination,
            context.aiAgentSettings().navigationAgent, ownerID);
        if (result.status != EnemyAiWorld::PathQueryStatus::Success || result.path.waypoints.empty()) {
            return EnemyCombatStatus::Failure;
        }
        m_pathFollower->SetPath(std::move(result.path), 0);
        m_hasPath = true;
    }
    m_pathFollower->Update(position);
    // 投影された経路終端に着いても指定位置に届かなければ、呼び出し側に再選択を委ねる。
    return !m_pathFollower->HasPath() || m_pathFollower->HasReachedGoal()
        ? EnemyCombatStatus::Failure : EnemyCombatStatus::Running;
}

void EnemyRePositionNavigation::Cancel()
{
    if (m_pathFollower) m_pathFollower->ClearPath();
    m_pathFollower = nullptr;
    m_hasPath = false;
}
