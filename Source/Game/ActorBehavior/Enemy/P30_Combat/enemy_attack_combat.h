#pragma once

#include "enemy_combat_base.h"

/// @brief 射程内で攻撃を担当するCombat。攻撃内容は敵種ごとに後から実装する。
class EnemyAttackCombat : public EnemyCombatBase {
    int m_locomotionRequestHandle = -1;
    float m_attackDistance = 2.0f;

public:
    bool CanStart(const EnemyContext& context) const override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Abort(EnemyContext& context) override;

    void SetAttackDistance(float value) { m_attackDistance = value; }

protected:
    virtual void UpdateAttack(EnemyContext& context, float deltaTime);
};
