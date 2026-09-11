//===================================================
// File  ：_/EnemyAI/attack_coordinator_system.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "attack_coordinator_system.h"

void AttackCoordinatorSystem::Initialize(const EnemyAIWorldContext& context) 
{
    m_waitingRequests.clear();
    m_currentAttacker.clear();
}

void AttackCoordinatorSystem::Update(EnemyAIWorldContext& context, float deltaTime) 
{
    
}

void AttackCoordinatorSystem::Finalize(const EnemyAIWorldContext& context) 
{
    m_waitingRequests.clear();
    m_currentAttacker.clear();
}

/// @brief 攻撃のリクエストを追加する
void AttackCoordinatorSystem::RequestAttack(const AttackRequest& request)
{
    m_waitingRequests.push_back(request);
}

/// @brief 指定された敵が攻撃可能かどうかを判定する
bool AttackCoordinatorSystem::CanAttack(int enemyId) const 
{
    auto it = std::find(m_currentAttacker.begin(), m_currentAttacker.end(), enemyId);
    if (it != m_currentAttacker.end()) {
        // 現在攻撃中の敵であれば攻撃可能
        return true;
    }
    return true;
}