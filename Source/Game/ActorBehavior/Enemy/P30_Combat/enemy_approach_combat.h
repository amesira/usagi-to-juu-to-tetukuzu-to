#pragma once

#include <cstddef>

#include "enemy_combat_base.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_context.h"

class EnemyApproachCombat : public EnemyCombatBase {
    EnemyAiWorld::NavigationPath m_path;
    size_t m_waypointIndex = 0;
    float m_repathTimer = 0.0f;
    int m_locomotionRequestHandle = -1;

    float m_moveSpeed = 6.0f;
    float m_repathInterval = 0.5f;
    float m_reachDistance = 0.2f;
    float m_attackDistance = 2.0f;

public:
    bool CanStart(const EnemyContext& context) const override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Abort(EnemyContext& context) override;

    void SetMoveSpeed(float value) { m_moveSpeed = value; }
    void SetAttackDistance(float value) { m_attackDistance = value; }

private:
    void UpdatePath(EnemyContext& context);
    void UpdateLocomotionRequest(EnemyContext& context);
    void ClearPath();
};
