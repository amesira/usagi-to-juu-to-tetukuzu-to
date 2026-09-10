//---------------------------------------------------
// File  ：_/EnemyAI/navigation_system.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・ナビゲーショングリッドの生成・経路探索を担当するクラス
//---------------------------------------------------
#ifndef NAVIGATION_SYSTEM_H
#define NAVIGATION_SYSTEM_H
#include "enemy_ai_world_context.h"
#include "Engine/Core/game_object_layer.h"

class IScene;
namespace EnemyAiAgent { struct NavigationAgentSettings; }

// ナビゲーショングリッドの生成・経路探索を担当する
class NavigationSystem {
    friend struct NavigationOptimizationTestAccess;
private:
    static constexpr CollisionLayerMask SAMPLE_LAYER_MASK = CollisionLayerToMask(CollisionLayer::Field);

    EnemyAiWorld::NavigationGridSettings m_buildSettings;
    bool m_isBuilt = false;

    // index = x * cellCountZ + z
    std::vector<EnemyAiWorld::GridCell> m_cells;

    /// @brief A*探索用のノード情報
    struct SearchNode {
        float gCost = std::numeric_limits<float>::infinity();
        int parentIndex = -1;
        bool closed = false; // 探索済み
    };

public:
    struct SearchStats {
        double milliseconds = 0;
        size_t expandedNodes = 0;
        size_t walkableEvaluations = 0;
        size_t walkableCacheHits = 0;
    };
    const SearchStats& GetLastSearchStats() const { return m_lastSearchStats; }
    void Initialize(EnemyAIWorldContext& context);
    void Finalize(EnemyAIWorldContext& context);

    /// @brief NavigationGridを生成する
    bool BuildGrid(EnemyAIWorldContext& context);
    void ClearGrid() {
        m_cells.clear();
        m_isBuilt = false;
    }

    // === NavigationCell変換・取得 ===
    bool WorldToGrid(const DirectX::XMFLOAT3& position, EnemyAiWorld::GridCoord& outCoord) const;
    bool GridToWorld(EnemyAiWorld::GridCoord coord, DirectX::XMFLOAT3& outPosition) const;
    const EnemyAiWorld::GridCell* GetCell(EnemyAiWorld::GridCoord coord) const;

    // === NavigationPath探索 ===
    bool IsWalkable(EnemyAiWorld::GridCoord coord,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;
    bool CanTraverse(EnemyAiWorld::GridCoord from, 
        EnemyAiWorld::GridCoord to,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;

    EnemyAiWorld::PathQueryResult FindPath(
        EnemyAIWorldContext& context,
        const DirectX::XMFLOAT3& start, 
        const DirectX::XMFLOAT3& goal,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;
    void SmoothPath(
        const EnemyAIWorldContext& context,
        const EnemyAiAgent::NavigationAgentSettings& agent, 
        EnemyAiWorld::NavigationPath& path) const;

    // === Debug用 ===
    void DrawDebugGrid() const;

private:
    mutable SearchStats m_lastSearchStats;
    bool CachedIsWalkable(EnemyAiWorld::GridCoord coord,
        const EnemyAiAgent::NavigationAgentSettings& agent,
        std::vector<signed char>* cache, SearchStats* stats) const;
    bool CanTraverseImpl(EnemyAiWorld::GridCoord from, EnemyAiWorld::GridCoord to,
        const EnemyAiAgent::NavigationAgentSettings& agent,
        std::vector<signed char>* cache, SearchStats* stats) const;
    /// @brief 1セルの地形を取得する
    EnemyAiWorld::GridCell SampleCell(
        class IScene* scene,
        const EnemyAiWorld::NavigationGridSettings& settings,
        EnemyAiWorld::GridCoord coord) const;

    /// @brief セル座標が有効かどうかを判定する
    bool ValidateCellCoord(EnemyAiWorld::GridCoord coord) const {
        return coord.x >= 0 && coord.x < m_buildSettings.cellCountX &&
            coord.z >= 0 && coord.z < m_buildSettings.cellCountZ;
    }

};

#endif // NAVIGATION_SYSTEM_H
