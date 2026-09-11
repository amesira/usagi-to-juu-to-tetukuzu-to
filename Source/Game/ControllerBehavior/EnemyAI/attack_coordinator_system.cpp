//===================================================
// File  ：_/EnemyAI/attack_coordinator_system.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "attack_coordinator_system.h"
#include "enemy_ai_world_context.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Game/ActorBehavior/Enemy/enemy_behavior.h"

using namespace EnemyAiWorld;

void AttackCoordinatorSystem::Initialize(const EnemyAIWorldContext& context)
{
    m_waitingRequests.clear();
    m_attackPermissions.clear();
    m_currentAttackers.clear();

    m_nextPermissionTime = 0.0f;
    m_initialized = true;

    m_enemyValidator = [scene = context.scene](int enemyId) {
        if (!scene) return false;
        GameObject* enemyObject = scene->GetGameObjectByID(static_cast<unsigned int>(enemyId));
        if (!enemyObject) return false;
        auto* behavior = enemyObject->GetComponent<EnemyBehavior>();
        if (!behavior) return false;
        return true;
        };
}

void AttackCoordinatorSystem::Finalize(const EnemyAIWorldContext& context)
{
    m_waitingRequests.clear();
    m_attackPermissions.clear();
    m_currentAttackers.clear();

    m_nextPermissionTime = 0.0f;
    m_enemyValidator = {};
    m_initialized = false;
}

void AttackCoordinatorSystem::Update(EnemyAIWorldContext& context, float deltaTime)
{
    if (deltaTime <= 0.0f) return;
    if (!m_initialized) return;

    // 次の許可までの時間を減算
    m_nextPermissionTime -= deltaTime;
    if (m_nextPermissionTime < 0.0f) m_nextPermissionTime = 0.0f;

    // 無効な敵のリクエストを削除
    std::erase_if(m_waitingRequests,[this](const auto& request) { 
            return !IsEnemyValid(request.enemyId);
        });

    for (auto& permission : m_attackPermissions) {
        permission.remainingTime -= deltaTime;
    }

    // 攻撃リクエストの有効時間が切れたもの、または無効な敵の許可を削除
    const auto expired = std::erase_if(m_attackPermissions, [this](const auto& permission) {
        return permission.remainingTime <= 0.0f || !IsEnemyValid(permission.enemyId);
        });
    const auto invalid = std::erase_if(m_currentAttackers,[this](const int& enemyId) { 
        return !IsEnemyValid(enemyId);
        });
    if (expired || invalid) DelayNextPermission(context);

    // 攻撃許可を出す
    while (!m_waitingRequests.empty() && m_nextPermissionTime <= 0.0f
        && m_attackPermissions.size() + m_currentAttackers.size()
        < static_cast<size_t>(context.settings().attackCoordinator.maxConcurrentAttacks)) 
    {
        const int enemyId = m_waitingRequests.front().enemyId;
        m_attackPermissions.push_back({ enemyId, context.settings().attackCoordinator.permissionLifetime});
        m_waitingRequests.pop_front();
        DelayNextPermission(context);
    }
}

bool AttackCoordinatorSystem::IsAttacking(int enemyId) const
{
    return std::find(m_currentAttackers.begin(), m_currentAttackers.end(), enemyId)
        != m_currentAttackers.end();
}

bool AttackCoordinatorSystem::HasAttackRequest(int enemyId) const
{
    return IsAttacking(enemyId)
        || std::any_of(m_waitingRequests.begin(), m_waitingRequests.end(),
            [enemyId](const auto& request) { return request.enemyId == enemyId; })
        || std::any_of(m_attackPermissions.begin(), m_attackPermissions.end(),
            [enemyId](const auto& permission) { return permission.enemyId == enemyId; });
}

bool AttackCoordinatorSystem::RequestAttack(const AttackRequest& request)
{
    if (!IsEnemyValid(request.enemyId) || HasAttackRequest(request.enemyId)) return false;
    m_waitingRequests.push_back(request);
    return true;
}

bool AttackCoordinatorSystem::CanAttack(int enemyId) const
{
    if (!IsEnemyValid(enemyId)) return false;
    return std::any_of(m_attackPermissions.begin(), m_attackPermissions.end(),
        [enemyId](const auto& permission) {
            return permission.enemyId == enemyId && permission.remainingTime > 0.0f;
        });
}

bool AttackCoordinatorSystem::ConsumeAttackRequest(int enemyId)
{
    // CanAttackから状態が変わっている場合もあるので、必ずここで再確認する。
    if (!IsEnemyValid(enemyId)) {
        CancelAttackRequest(enemyId);
        return false;
    }

    const auto it = std::find_if(m_attackPermissions.begin(), m_attackPermissions.end(),
        [enemyId](const auto& permission) {
            return permission.enemyId == enemyId && permission.remainingTime > 0.0f;
        });
    if (it == m_attackPermissions.end()) return false;
    m_currentAttackers.push_back(enemyId);
    m_attackPermissions.erase(it);
    return true;
}

/// @brief 次の攻撃許可を出すまでの時間を遅延させる
void AttackCoordinatorSystem::DelayNextPermission(const EnemyAIWorldContext& context)
{
    m_nextPermissionTime = (std::max)(m_nextPermissionTime, context.settings().attackCoordinator.attackInterval);
}

bool AttackCoordinatorSystem::IsEnemyValid(int enemyId) const
{
    return m_enemyValidator ? m_enemyValidator(enemyId) : false;
}

/// @brief 攻撃リクエストをキャンセルする
bool AttackCoordinatorSystem::CancelAttackRequest(int enemyId)
{
    const auto waiting = std::erase_if(m_waitingRequests,
        [enemyId](const auto& request) { return request.enemyId == enemyId; });
    const auto permitted = std::erase_if(m_attackPermissions,
        [enemyId](const auto& permission) { return permission.enemyId == enemyId; });
    const auto attacking = std::erase(m_currentAttackers, enemyId);

    return waiting || permitted || attacking;
}
