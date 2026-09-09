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
private:
    static constexpr CollisionLayerMask SAMPLE_LAYER_MASK = static_cast<CollisionLayerMask>(CollisionLayer::Field);

    EnemyAiWorld::NavigationGridSettings m_buildSettings;
    bool m_isBuilt = false;

    // index = z * cellCountX + x
    std::vector<EnemyAiWorld::GridCell> m_cells;

public:
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

    // 地面・傾斜・半径分の余白を、このAgentの条件で判定する。
    bool IsWalkable(const EnemyAIWorldContext& context, EnemyAiWorld::GridCoord coord,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;
    // 隣接セルへの移動可否。段差と斜め移動の角抜けも判定する。
    bool CanTraverse(const EnemyAIWorldContext& context,
        EnemyAiWorld::GridCoord from, EnemyAiWorld::GridCoord to,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;

    // 8方向A*でWorld座標の経由点を返す。探索作業データは呼び出しごとに保持。
    EnemyAiWorld::PathQueryResult FindPath(const EnemyAIWorldContext& context,
        const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& goal,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;
    // 地面・傾斜・段差・半径の条件を満たす区間だけ中間点を省く。
    void SmoothPath(const EnemyAIWorldContext& context,
        const EnemyAiAgent::NavigationAgentSettings& agent, EnemyAiWorld::NavigationPath& path) const;

    // === Debug用 ===
    void DrawDebugGrid() const;

private:
    // 1セルの地形を取得。地面なしはNoGround、Agent別の傾斜判定は行わない。
    EnemyAiWorld::GridCell SampleCell(const EnemyAIWorldContext& context,
        const EnemyAiWorld::NavigationGridSettings& settings, EnemyAiWorld::GridCoord coord) const;
    // 平滑化用の区間判定。Rayの見通しだけでなく途中の地形と余白も確認する。
    bool CanMoveDirectly(const EnemyAIWorldContext& context,
        const DirectX::XMFLOAT3& from, const DirectX::XMFLOAT3& to,
        const EnemyAiAgent::NavigationAgentSettings& agent) const;

};

#endif // NAVIGATION_SYSTEM_H
