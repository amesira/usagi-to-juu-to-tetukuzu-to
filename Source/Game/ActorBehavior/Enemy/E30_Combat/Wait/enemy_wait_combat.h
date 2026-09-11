//---------------------------------------------------
// File  ：_/E30_Combat/Wait/enemy_wait_combat.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・攻撃前の待機。RequestWaitで予約し、完了後はIsCompletedを攻撃側で確認する
// ・他の敵との攻撃タイミングが重なってしまうのを防ぎたい
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

class EnemyLocomotionController;

/// @brief 攻撃前の待機。RequestWaitで予約し、完了後はIsCompletedを攻撃側で確認する
class EnemyWaitCombat : public EnemyCombatBase {
    EnemyLocomotionController* m_controller = nullptr;
    int m_requestHandle = -1;

    float m_waitDuration = 0.3f;
    float m_remainingTime = 0.0f;
    float m_minDistance = 0.0f;
    float m_maxDistance = 2.0f;
    
    bool m_requested = false;
    bool m_completed = false;

public:
    EnemyWaitCombat() : EnemyCombatBase(15, true) {}

    bool IsCompleted() const { return m_completed; }
    float GetRemainingTime() const { return m_remainingTime; }

    bool CanStart(const EnemyContext& context) const override;
    bool CanContinue(const EnemyContext& context) const override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

private:
    bool UpdateLocomotion(const EnemyContext& context);
    void ReleaseLocomotion();

};
