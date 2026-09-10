//===================================================
// File  ：_/EnemyAI/tactical_query_system.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include "tactical_query_system.h"

#include "meta_ai.h"
#include "navigation_system.h"

void TacticalQuerySystem::Initialize(const EnemyAIWorldContext& context)
{
    m_tacticalCells.clear();
    m_tacticalCells.resize(context.settings().navigationGrid.cellCountX * context.settings().navigationGrid.cellCountZ);

    m_updateTimer = 0.0f;
}

void TacticalQuerySystem::Update(EnemyAIWorldContext& context, float deltaTime)
{
    if (m_updateTimer < UPDATE_INTERVAL) {
        m_updateTimer += deltaTime;
        return;
    }
    m_updateTimer = 0.0f;

    // タクティカルセル情報をリセット
    for (auto& cell : m_tacticalCells) {
        cell.enemyDensityCost = 0.0f;
        cell.reservationCost = 0.0f;
    }

    // 敵の密度を評価
    auto& enemies = context.metaAI->GetEnemies();
    for (const auto& enemy : enemies) {
        EnemyAiWorld::GridCoord coord;
        if (!context.navigation->WorldToGrid(enemy.entityInfo.position, coord)) continue;
        
        int index = coord.x * context.settings().navigationGrid.cellCountZ + coord.z;
        if (index >= 0 && index < m_tacticalCells.size()) {
            m_tacticalCells[index].enemyDensityCost += ENEMY_DENSITY_COST_WEIGHT;
        }
    }

    // 他の敵が予約しているセルのコストを評価する処理を追加する場合は、ここに実装
}

void TacticalQuerySystem::Finalize(const EnemyAIWorldContext& context)
{
    m_tacticalCells.clear();
}

/// @brief 指定されたセル座標のタクティカルコストを取得する
float TacticalQuerySystem::GetTacticalCost(const EnemyAIWorldContext& context, const EnemyAiWorld::GridCoord& coord)
{
    int index = coord.x * context.settings().navigationGrid.cellCountZ + coord.z;
    if (index >= 0 && index < m_tacticalCells.size()) {
        const auto& cell = m_tacticalCells[index];
        return cell.enemyDensityCost + cell.reservationCost;
    }

    return 0.0f;
}
