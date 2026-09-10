//===================================================
// File  ：_/EnemyAI/tactical_query_system.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include "tactical_query_system.h"

#include "meta_ai.h"
#include "navigation_system.h"

#include "Utility/mi_math.h"

using namespace EnemyAiWorld;

namespace {
    /// @brief 敵の密度コストを計算する
    float CalculateDensityCost(float distance, float radius, float strength) {
        if (radius <= 0.0f) return 0.0f;

        const float influence = (std::max)(0.0f, 1.0f - distance / radius);
        return influence * strength;
    }
}

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
    m_densitySources.clear();

    // 敵の密度を評価
    auto& enemies = context.metaAI->GetEnemies();
    for (const auto& enemy : enemies) {
        GridCoord coord;
        if (!context.navigation->WorldToGrid(enemy.entityInfo.position, coord)) continue;
        float radius = enemy.entityInfo.radius * 5.0f; // 敵の影響範囲を半径1.5倍に設定

        // 敵の密度源情報を更新
        m_densitySources[enemy.entityInfo.gameObjectID] = { 
            enemy.entityInfo.position, 
            radius, 
            ENEMY_DENSITY_COST_WEIGHT
        };

        // radius内の周囲セルにも影響を与える
        int cellRadius = static_cast<int>(std::ceil(radius / context.settings().navigationGrid.cellSize));
        for (int dx = -cellRadius; dx <= cellRadius; ++dx) {
            for (int dz = -cellRadius; dz <= cellRadius; ++dz) {

                GridCoord neighborCoord = { coord.x + dx, coord.z + dz };
                if (!context.navigation->ValidateCellCoord(neighborCoord)) continue;

                XMFLOAT3 cellPosition;
                context.navigation->GridToWorld(neighborCoord, cellPosition);

                float distance = std::hypot(
                    enemy.entityInfo.position.x - cellPosition.x,
                    enemy.entityInfo.position.z - cellPosition.z);
                const float densityCost = CalculateDensityCost(distance, radius, ENEMY_DENSITY_COST_WEIGHT);

                int neighborIndex = neighborCoord.x * context.settings().navigationGrid.cellCountZ + neighborCoord.z;
                if (neighborIndex >= 0 && neighborIndex < m_tacticalCells.size()) {
                    m_tacticalCells[neighborIndex].enemyDensityCost += densityCost;
                }
            }
        }
    }

    // 他の敵が予約しているセルのコストを評価する処理を追加する場合は、ここに実装
}

void TacticalQuerySystem::Finalize(const EnemyAIWorldContext& context)
{
    m_tacticalCells.clear();
}

/// @brief 指定されたセル座標のタクティカルコストを取得する
float TacticalQuerySystem::GetTacticalCost(const EnemyAIWorldContext& context, 
    const EnemyAiWorld::GridCoord& coord, int enemyId)
{
    if (!context.navigation->ValidateCellCoord(coord)) return 0.0f;

    const int index = coord.x * context.settings().navigationGrid.cellCountZ + coord.z;
    if (index < 0 || static_cast<size_t>(index) >= m_tacticalCells.size()) {
        return 0.0f;
    }

    const auto& cell = m_tacticalCells[index];
    float densityCost = cell.enemyDensityCost;

    if (enemyId >= 0) {
        const auto it = m_densitySources.find(enemyId);
        if (it != m_densitySources.end()) {
            const auto& source = it->second;

            XMFLOAT3 cellPosition;
            if (context.navigation->GridToWorld(coord, cellPosition)) {
                const float distance = std::hypot(
                    source.position.x - cellPosition.x,
                    source.position.z - cellPosition.z);

                // 自身の影響を除外するため、密度コストを減算する
                densityCost -= CalculateDensityCost(distance, source.influenceRadius, source.strength);
            }
        }
    }

    return (std::max)(0.0f, densityCost) + cell.reservationCost;
}
