//---------------------------------------------------
// File  ：_/E30_Combat/Approach/enemy_approach_combat.h
// Date  ：2026/09/10
// Author：Miu Kitamura
//
// ・ターゲットまでの再探索を要求し、PathFollowerの方向をLocomotionへ渡す。
//---------------------------------------------------
#pragma once

#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

class EnemyApproachCombat : public EnemyCombatBase {
    float m_repathTimer = 0.0f;
    int m_locomotionRequestHandle = -1;

    float m_repathInterval = 0.5f;
    float m_attackDistance = 2.0f;

public:
    bool CanStart(const EnemyContext& context) const override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

    void SetAttackDistance(float value) { m_attackDistance = value; }

private:
    void UpdatePath(EnemyContext& context);
    void UpdateLocomotionRequest(EnemyContext& context);
};
