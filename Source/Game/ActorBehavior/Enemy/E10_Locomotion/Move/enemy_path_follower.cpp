#include "enemy_path_follower.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Utility/mi_math.h"

void EnemyPathFollower::Initialize(float reachDistance)
{
    m_reachDistance = (std::max)(0.0f, reachDistance);
    ClearPath();
}

void EnemyPathFollower::SetPath(EnemyAiWorld::NavigationPath path, size_t firstWaypointIndex)
{
    m_path = std::move(path);
    m_waypointIndex = (std::min)(firstWaypointIndex, m_path.waypoints.size());
    m_moveDirection = {};
}

void EnemyPathFollower::ClearPath()
{
    m_path.waypoints.clear();
    m_waypointIndex = 0;
    m_moveDirection = {};
}

void EnemyPathFollower::Update(const DirectX::XMFLOAT3& currentPosition)
{
    m_moveDirection = {};

    while (m_waypointIndex < m_path.waypoints.size()) {
        const auto& waypoint = m_path.waypoints[m_waypointIndex];
        DirectX::XMFLOAT3 difference = {
            waypoint.x - currentPosition.x,
            0.0f,
            waypoint.z - currentPosition.z,
        };

        if (std::hypot(difference.x, difference.z) <= m_reachDistance) {
            ++m_waypointIndex;
            continue;
        }

        m_moveDirection = MiMath::Normalize(difference);
        return;
    }
}
