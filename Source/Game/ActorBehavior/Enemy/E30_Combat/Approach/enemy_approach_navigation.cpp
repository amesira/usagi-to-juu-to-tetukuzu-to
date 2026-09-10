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

    context.runtimeState = {};
    context.runtimeState.stuckCheckTimer = context.settings().stuckCheckInterval;

    // 経路再探索の進捗監視用に、現在位置を保存する
    if (context.enemyTransform) {
        context.runtimeState.lastProgressPosition = context.enemyTransform->GetPosition();
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
    state.destination = context.enemyRuntimeState->combatTargetPosition;
    state.hasReachedDestination = Distance(position, state.destination) <= settings.stopDistance;
    if (state.hasReachedDestination) {
        return EnemyCombatStatus::Success;
    }

    // 経路再探索のタイマーを更新する
    state.repathTimer -= deltaTime;
    state.minRepathTimer -= deltaTime;
    state.retryTimer -= deltaTime;

    // 経路追従の更新を行う
    context.pathFollower->Update(position);

    const auto direction = context.pathFollower->GetMoveDirection();
    if (MiMath::Length(direction) > 0.001f) {
        state.stuckCheckTimer -= deltaTime;

        if (state.stuckCheckTimer <= 0.0f) {
            if (Distance(position, state.lastProgressPosition) < settings.minProgressDistance) {
                if (state.consecutiveStuckChecks++ >= settings.maxPathFailures) {
                    return EnemyCombatStatus::Failure;
                }
                state.repathTimer = 0.0f;
            }
            else {
                state.consecutiveStuckChecks = 0;
            }

            // 進捗があった場合は、現在位置を保存してタイマーをリセットする
            state.lastProgressPosition = position;
            state.stuckCheckTimer = settings.stuckCheckInterval;
        }
    }
    else {
        state.lastProgressPosition = position;
        state.stuckCheckTimer = settings.stuckCheckInterval;
    }

    // === タイマーやターゲットの移動距離に応じて経路再探索を要求する ===
    const bool targetMoved = Distance(state.destination, state.lastPathTargetPosition) >= settings.targetMoveThreshold;
    const bool needRepath = !context.pathFollower->HasPath() || context.pathFollower->HasReachedGoal();
    const bool canRepath = state.repathTimer <= 0.0f || targetMoved || needRepath;

    if (state.retryTimer <= 0.0f && state.minRepathTimer <= 0.0f && needRepath && canRepath) {
        auto result = context.aiWorld->FindPath(position, state.destination, context.aiAgentSettings().navigationAgent);
        
        // 経路探索の結果を保存し、再探索のタイマーをリセットする
        state.lastPathTargetPosition = state.destination;
        state.repathTimer = settings.repathInterval;
        state.minRepathTimer = settings.minRepathInterval;
        context.pathFollower->ClearPath();

        if (result.status != EnemyAiWorld::PathQueryStatus::Success || result.path.waypoints.empty()) {
            state.retryTimer = settings.pathRetryInterval;
            if (state.consecutivePathFailures++ >= settings.maxPathFailures) {
                return EnemyCombatStatus::Failure;
            }
        }
        else {
            // FindPathの[start, 始点セル中心, ..., goal]の先頭をスキップする。
            const size_t firstWaypoint = result.path.waypoints.size() > 2 ? 2 : 0;
            context.pathFollower->SetPath(std::move(result.path), firstWaypoint);
            context.pathFollower->Update(position);

            // 投影先が現在位置の場合など、経路終端でも接近できていないケース
            if (context.pathFollower->HasReachedGoal()) {
                context.pathFollower->ClearPath();
                state.retryTimer = settings.pathRetryInterval;
                if (state.consecutivePathFailures++ >= settings.maxPathFailures) return EnemyCombatStatus::Failure;
            } 
            else {
                state.consecutivePathFailures = 0;
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
