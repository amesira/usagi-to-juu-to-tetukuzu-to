#pragma once

#include <cstddef>
#include <DirectXMath.h>

#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_context.h"

/// @brief NavigationPathの経由点を進め、現在の移動方向を提供する。
class EnemyPathFollower {
    EnemyAiWorld::NavigationPath m_path;
    size_t m_waypointIndex = 0;
    float m_reachDistance = 0.2f;
    DirectX::XMFLOAT3 m_moveDirection = {};

public:
    void Initialize(float reachDistance);
    void SetPath(EnemyAiWorld::NavigationPath path, size_t firstWaypointIndex = 0);
    void ClearPath();
    void Update(const DirectX::XMFLOAT3& currentPosition);

    bool HasPath() const { return !m_path.waypoints.empty(); }
    bool HasReachedGoal() const { return HasPath() && m_waypointIndex >= m_path.waypoints.size(); }
    const DirectX::XMFLOAT3& GetMoveDirection() const { return m_moveDirection; }
    size_t GetWaypointIndex() const { return m_waypointIndex; }
    const EnemyAiWorld::NavigationPath& GetPath() const { return m_path; }
};
