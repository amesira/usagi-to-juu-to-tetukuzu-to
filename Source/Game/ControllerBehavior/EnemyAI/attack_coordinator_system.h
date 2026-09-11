//---------------------------------------------------
// File  ：_/EnemyAI/attack_coordinator_system.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・敵の攻撃行動を調整するシステム
//---------------------------------------------------
#pragma once
#include <vector>
#include <deque>
#include <DirectXMath.h>
#include "enemy_ai_world_context.h"

class AttackCoordinatorSystem {
public:
    struct AttackRequest {
        int enemyId = -1;
    };

private:
    // 待機中の攻撃リクエスト
    std::deque<AttackRequest> m_waitingRequests;

    // 現在攻撃中の敵
    std::vector<int> m_currentAttacker;

public:
    void Initialize(const EnemyAIWorldContext& context);
    void Update(EnemyAIWorldContext& context, float deltaTime);
    void Finalize(const EnemyAIWorldContext& context);

    void RequestAttack(const AttackRequest& request);
    bool CanAttack(int enemyId) const;

};