#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_combat.h"

class EnemyRangedAttackCombat : public EnemyAttackCombat {
    int m_shotsFired = 0;
    float m_timeUntilNextShot = 0.0f;

public:
    int GetShotsFired() const { return m_shotsFired; }
    float GetTimeUntilNextShot() const { return m_timeUntilNextShot; }

protected:
    void BeginAttack(EnemyContext& context) override;
    EnemyCombatStatus UpdateAttack(EnemyContext& context, float deltaTime) override;
    void EndAttack(EnemyContext& context) override;
    // 発射ごとに狙い直す。弾生成は未実装。
    virtual void FireShot(EnemyContext& context, const DirectX::XMFLOAT3& targetPosition);
    virtual void ClearAttackEffects(EnemyContext& context);
};
