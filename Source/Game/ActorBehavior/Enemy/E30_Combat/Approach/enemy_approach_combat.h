//---------------------------------------------------
// File  ：_/E30_Combat/Approach/enemy_approach_combat.h
// Date  ：2026/09/10
// Author：Miu Kitamura
//
// ・ターゲットまでの再探索を要求し、PathFollowerの方向をLocomotionへ渡す。
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"
#include "enemy_approach_context.h"

class EnemyApproachCombat : public EnemyCombatBase {
    EnemyApproachContext m_context;
    float m_restartCooldown = 0.0f;

public:
    EnemyApproachCombat() : EnemyCombatBase(10, true) {}

    void Initialize(const EnemyContext& context, const EnemyApproachSettingsAsset* settings);
    bool CanStart(const EnemyContext& context) const override;
    bool CanContinue(const EnemyContext& context) const override;
    bool IsInterruptible(const EnemyContext& context) const override;
    
    void UpdateBackground(EnemyContext& context, float deltaTime) override;
    
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

    float GetStopDistance() const { return m_context.settings().stopDistance; }
    const EnemyApproachContext& GetContext() const { return m_context; }

private:

};
