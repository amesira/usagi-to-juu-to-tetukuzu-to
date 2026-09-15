//---------------------------------------------------
// File  ：_/EnemyAI/attack_coordinator_system.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・敵の攻撃を調整するシステム
//---------------------------------------------------
#pragma once
#include <deque>
#include <vector>
#include <functional>

struct EnemyAIWorldContext;

class AttackCoordinatorSystem {
public:
    struct AttackRequest { 
        int enemyId = -1;
    };
    struct AttackPermission {
        int enemyId = -1;
        float remainingTime = 0.0f;
    };

    

private:
    using EnemyValidator = std::function<bool(int enemyId)>;
    EnemyValidator m_enemyValidator;

    std::deque<AttackRequest> m_waitingRequests;
    std::vector<AttackPermission> m_attackPermissions;
    std::vector<int> m_currentAttackers;

    // 次の攻撃許可を出すまでの残り時間
    float m_nextPermissionTime = 0.0f;
    bool m_initialized = false;
    const EnemyAIWorldContext* m_worldContext = nullptr;

public:
    void Initialize(const EnemyAIWorldContext& context);
    void Update(EnemyAIWorldContext& context, float deltaTime);
    void Finalize(const EnemyAIWorldContext& context);

    bool RequestAttack(const AttackRequest& request);
    bool CanAttack(int enemyId) const;
    bool ConsumeAttackRequest(int enemyId);
    bool FinishAttack(int enemyId);

    bool CancelAttackRequest(int enemyId);
    bool HasAttackRequest(int enemyId) const;
    bool IsAttacking(int enemyId) const;

    const std::deque<AttackRequest>& GetWaitingRequests() const { return m_waitingRequests; }
    const std::vector<AttackPermission>& GetAttackPermissions() const { return m_attackPermissions; }
    const std::vector<int>& GetCurrentAttackers() const { return m_currentAttackers; }

private:
    void DelayNextPermission(const EnemyAIWorldContext& context);
    bool IsEnemyValid(int enemyId) const;

};
