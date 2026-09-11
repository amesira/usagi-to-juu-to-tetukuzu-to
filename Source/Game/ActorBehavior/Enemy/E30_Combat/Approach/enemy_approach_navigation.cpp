//===================================================
// File  ：_/E30_Combat/Approach/enemy_approach_navigation.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_approach_navigation.h"
#include "enemy_approach_context.h"

#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/Move/enemy_path_follower.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"

#include <cmath>
#include <utility>

namespace {
    float Distance(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
        return std::hypot(a.x - b.x, a.z - b.z);
    }
}

void EnemyApproachNavigation::Start(EnemyApproachContext& context)
{
    Cancel(context);

    context.runtimeState.hasReachedDestination = false;
    m_lastPathTargetPosition = {};
    m_lastProgressPosition = {};
    
    // 経路再探索のタイマーをランダム化して、複数の敵が同時に経路探索を行わないようにしてみる
    int r = std::rand() % 10;
    m_repathTimer = context.settings().repathInterval * (static_cast<float>(r) / 10.0f);
    m_minRepathTimer = context.settings().minRepathInterval * (static_cast<float>(r) / 10.0f);

    m_retryTimer = 0.0f;
    m_consecutivePathFailures = 0;
    m_consecutiveStuckChecks = 0;
    m_stuckCheckTimer = context.settings().stuckCheckInterval;

    // 経路再探索の進捗監視用に、現在位置を保存する
    if (context.enemyTransform) {
        m_lastProgressPosition = context.enemyTransform->GetPosition();
    }
}

EnemyCombatStatus EnemyApproachNavigation::Update(EnemyApproachContext& context, float deltaTime)
{
    if (deltaTime <= 0.0f) return EnemyCombatStatus::Running;

    if (!context.enemyTransform || !context.enemyRuntimeState || !context.pathFollower
        || !context.aiWorld || !context.aiWorld->GetEnable() || !context.aiWorld->IsInitialized()) {
        return EnemyCombatStatus::Failure;  // AIWorldが利用できない場合は失敗扱いで終了する
    }

    auto& state = context.runtimeState;
    const auto& settings = context.settings();
    const auto position = context.enemyTransform->GetPosition();

    // ターゲットの位置を目的地として設定する
    const auto destination = context.enemyRuntimeState->combatTargetPosition;
    state.hasReachedDestination = Distance(position, destination) <= settings.stopDistance;
    if (state.hasReachedDestination) {
        return EnemyCombatStatus::Success;
    }

    // 経路再探索のタイマーを更新する
    m_repathTimer -= deltaTime;
    m_minRepathTimer -= deltaTime;
    m_retryTimer -= deltaTime;

    context.pathFollower->Update(position);

    // === 立ち往生検知 ===
    const auto direction = context.pathFollower->GetMoveDirection();
    if (MiMath::Length(direction) > 0.001f) {
        m_stuckCheckTimer -= deltaTime;

        if (m_stuckCheckTimer <= 0.0f) {
            if (Distance(position, m_lastProgressPosition) < settings.minProgressDistance) {
                if (++m_consecutiveStuckChecks >= settings.maxPathFailures) {
                    return EnemyCombatStatus::Failure;
                }
                m_repathTimer = 0.0f;
            }
            else {
                m_consecutiveStuckChecks = 0;
            }

            // 進捗があった場合は、現在位置を保存してタイマーをリセットする
            m_lastProgressPosition = position;
            m_stuckCheckTimer = settings.stuckCheckInterval;
        }
    }
    else {
        m_lastProgressPosition = position;
        m_stuckCheckTimer = settings.stuckCheckInterval;
    }

    // === タイマーやターゲットの移動距離に応じて経路再探索を要求する ===
    const bool targetMoved = Distance(destination, m_lastPathTargetPosition) >= settings.targetMoveThreshold;
    const bool needRepath = !context.pathFollower->HasPath() || context.pathFollower->HasReachedGoal();
    const bool canRepath = m_repathTimer <= 0.0f || targetMoved || needRepath;

    if (m_retryTimer <= 0.0f && m_minRepathTimer <= 0.0f && canRepath) {
        const auto& navigation = context.aiWorld->GetNavigationSystem();
        FailedPathQuery query;
        query.gridRevision = navigation.GetGridRevision();
        query.agent = context.aiAgentSettings().navigationAgent;
        query.valid = navigation.WorldToGrid(position, query.startCell)
            && navigation.WorldToGrid(destination, query.goalCell);
        // 到達不能と確認済みなら全探索を繰り返さない。
        if (m_failedPathQuery.Matches(query)) {
            context.pathFollower->ClearPath();
            return EnemyCombatStatus::Failure;
        }
        m_failedPathQuery.valid = false;
        auto result = context.aiWorld->FindPath(
            position, destination, context.aiAgentSettings().navigationAgent, context.gameObjectID);
        
        // 経路探索の結果を保存し、再探索のタイマーをリセットする
        m_lastPathTargetPosition = destination;
        m_repathTimer = settings.repathInterval;
        m_minRepathTimer = settings.minRepathInterval;
        context.pathFollower->ClearPath();

        // 完全探索での到達不能、またはセルの歩行不可だけを記録する。
        // 範囲外でセル座標を得られない場合・未準備・立ち往生は記録しない。
        if (query.valid && (result.status == EnemyAiWorld::PathQueryStatus::Unreachable
            || result.status == EnemyAiWorld::PathQueryStatus::InvalidStart
            || result.status == EnemyAiWorld::PathQueryStatus::InvalidGoal)) {
            m_failedPathQuery = query;
            return EnemyCombatStatus::Failure;
        }

        if (result.status != EnemyAiWorld::PathQueryStatus::Success || result.path.waypoints.empty()) {
            m_retryTimer = settings.pathRetryInterval;
            if (++m_consecutivePathFailures >= settings.maxPathFailures) {
                return EnemyCombatStatus::Failure;
            }
        }
        else {
            context.pathFollower->SetPath(std::move(result.path), 0);
            context.pathFollower->Update(position);

            // 投影先が現在位置の場合など、経路終端でも接近できていないケース
            if (context.pathFollower->HasReachedGoal()) {
                context.pathFollower->ClearPath();
                m_retryTimer = settings.pathRetryInterval;
                if (++m_consecutivePathFailures >= settings.maxPathFailures) return EnemyCombatStatus::Failure;
            } 
            else {
                m_consecutivePathFailures = 0;
            }
        }
    }

    return EnemyCombatStatus::Running;
}

void EnemyApproachNavigation::Cancel(EnemyApproachContext& context)
{
    if (context.pathFollower) {
        context.pathFollower->ClearPath();
    }
}
