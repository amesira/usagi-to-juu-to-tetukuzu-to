//---------------------------------------------------
// File  ：_/E30_Combat/Approach/enemy_approach_navigation.h
// Date  ：2026/09/10
// Author：Miu Kitamura
//
// ・PathFollowerの経路再探索を要求し、進捗を監視する
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_context.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

struct EnemyApproachContext;

class EnemyApproachNavigation {
private:
    struct FailedPathQuery {
        bool valid = false;
        EnemyAiWorld::GridCoord startCell;
        EnemyAiWorld::GridCoord goalCell;
        std::uint64_t gridRevision = 0;
        EnemyAiAgent::NavigationAgentSettings agent;

        bool Matches(const FailedPathQuery& other) const {
            return valid && other.valid && gridRevision == other.gridRevision
                && startCell.x == other.startCell.x && startCell.z == other.startCell.z
                && goalCell.x == other.goalCell.x && goalCell.z == other.goalCell.z
                && agent.radius == other.agent.radius
                && agent.maxStepHeight == other.agent.maxStepHeight
                && agent.maxSlopeDegrees == other.agent.maxSlopeDegrees;
        }
    };
    // Start/Cancelをまたいで保持する。Tactical密度は通行可能性を変えない。
    FailedPathQuery m_failedPathQuery;
    DirectX::XMFLOAT3 m_lastPathTargetPosition = {};
    DirectX::XMFLOAT3 m_lastProgressPosition = {};

    float m_repathTimer = 0.0f;
    float m_minRepathTimer = 0.0f;
    float m_retryTimer = 0.0f;
    float m_stuckCheckTimer = 0.0f;
    
    int m_consecutivePathFailures = 0;
    int m_consecutiveStuckChecks = 0;

public:
    void Start(EnemyApproachContext& context);
    EnemyCombatStatus Update(EnemyApproachContext& context, float deltaTime);
    void Cancel(EnemyApproachContext& context);

};
