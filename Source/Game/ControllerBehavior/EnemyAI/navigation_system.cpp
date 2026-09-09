//===================================================
// File  ：_/EnemyAI/navigation_system.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include "navigation_system.h"
#include "enemy_ai_agent_settings_asset.h"

#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"

#include "Utility/debug_renderer.h"
#include "Utility/mi_math.h"

using namespace DirectX;
using namespace EnemyAiWorld;
using namespace EnemyAiAgent;

void NavigationSystem::Initialize(EnemyAIWorldContext& context)
{
    m_isBuilt = false;
    BuildGrid(context);
}

void NavigationSystem::Finalize(EnemyAIWorldContext& context)
{
    ClearGrid();
}

/// @brief ナビゲーショングリッドを生成する
bool NavigationSystem::BuildGrid(EnemyAIWorldContext& context)
{
    if (m_isBuilt) return true;

    m_isBuilt = true;
    m_buildSettings = context.settings().navigationGrid;

    for (int x = 0; x < m_buildSettings.cellCountX; x++) {
        for (int z = 0; z < m_buildSettings.cellCountZ; z++) {
            GridCoord coord{ x, z };
            GridCell cell = SampleCell(context.scene, m_buildSettings, coord);
            m_cells.push_back(cell);
        }
    }
    return true;
}

#pragma region NavigationCell変換
/// @brief World座標XZをセル座標へ変換する。範囲外・未生成ならfalse
bool NavigationSystem::WorldToGrid(const DirectX::XMFLOAT3& position, 
    EnemyAiWorld::GridCoord& outChoord) const
{
    if (!m_isBuilt) return false;

    outChoord.x = static_cast<int>((position.x - m_buildSettings.origin().x) / m_buildSettings.cellSize);
    outChoord.z = static_cast<int>((position.z - m_buildSettings.origin().y) / m_buildSettings.cellSize);
    if (!ValidateCellCoord(outChoord)) return false;
    return true;
}

/// @brief セル中心XZと保存された地面高さYを返す。地面なし等はfalse
bool NavigationSystem::GridToWorld(EnemyAiWorld::GridCoord coord,
    DirectX::XMFLOAT3& outPosition) const
{
    if (!m_isBuilt) return false;
    if (!ValidateCellCoord(coord)) return false;

    outPosition = {
        m_buildSettings.origin().x + (coord.x + 0.5f) * m_buildSettings.cellSize,
        0.0f,
        m_buildSettings.origin().y + (coord.z + 0.5f) * m_buildSettings.cellSize
    };

    GridCell cell = m_cells[coord.x * m_buildSettings.cellCountZ + coord.z];
    if (cell.type == CellType::NoGround || cell.type == CellType::Unknown) {
        return true;
    }

    outPosition.y = cell.height;
    return true;
}

const EnemyAiWorld::GridCell* NavigationSystem::GetCell(EnemyAiWorld::GridCoord coord) const
{
    if (!m_isBuilt) return nullptr;
    if (!ValidateCellCoord(coord)) return nullptr;

    return &m_cells[coord.x * m_buildSettings.cellCountZ + coord.z];
}
#pragma endregion

#pragma region NavigationPath探索
/// @brief このセルが歩行可能か判定する
bool NavigationSystem::IsWalkable(EnemyAiWorld::GridCoord coord, 
    const EnemyAiAgent::NavigationAgentSettings& agent) const
{
    // FIX: 敵の半径なども考慮して、隣接セルの高さ差や斜面角度を判定する必要がある

    if (!m_isBuilt) return false;
    if (!ValidateCellCoord(coord)) return false;

    GridCell cell = m_cells[coord.x * m_buildSettings.cellCountZ + coord.z];
    if (cell.type == CellType::Ground) {
        return true;
    }
    else if (cell.type == CellType::SteepSlope) {
        float minUpDot = cosf(XMConvertToRadians(agent.maxSlopeDegrees));
        return cell.normal.y >= minUpDot;
    }

    return false;
}

/// @brief 隣接セルへの移動可否を判定する。段差と斜め移動の角抜けも確認する
bool NavigationSystem::CanTraverse(
    EnemyAiWorld::GridCoord from, 
    EnemyAiWorld::GridCoord to,
    const EnemyAiAgent::NavigationAgentSettings& agent) const
{
    if (!m_isBuilt) return false;
    if (!ValidateCellCoord(from) || !ValidateCellCoord(to)) return false;

    int dx = to.x - from.x;
    int dz = to.z - from.z;
    if (abs(dx) > 1 || abs(dz) > 1 || (dx == 0 && dz == 0)) {
        return false; // 隣接セルではない
    }
    if (!IsWalkable(from, agent) || !IsWalkable(to, agent)) {
        return false; // どちらかのセルが歩行不可
    }

    GridCell fromCell = m_cells[from.x * m_buildSettings.cellCountZ + from.z];
    GridCell toCell = m_cells[to.x * m_buildSettings.cellCountZ + to.z];
    
    // === 斜め移動の場合 ===
    if (dx != 0 && dz != 0) {
        GridCoord adj1 = { from.x + dx, from.z };
        GridCoord adj2 = { from.x, from.z + dz };

        // 直線的に隣接する2つのセルを、再帰的に確認する
        bool canAdjTraverse = CanTraverse(from, adj1, agent);
        canAdjTraverse &= CanTraverse(from, adj2, agent);
        canAdjTraverse &= CanTraverse(adj1, to, agent);
        canAdjTraverse &= CanTraverse(adj2, to, agent);
        return canAdjTraverse;
    }
    // === 直線移動の場合 ===
    else {
        // 段差が許容範囲内なら、セルタイプに関わらず移動可能とする
        if (abs(toCell.height - fromCell.height) <= agent.maxStepHeight) {
            return true;
        }

        // 斜面セル同士の移動は、法線と移動方向の内積を確認してほぼ0であれば
        // 斜面の角度が移動方向に対してほぼ水平だと判断できるはず
        if (fromCell.type == CellType::SteepSlope && toCell.type == CellType::SteepSlope) {
            XMFLOAT3 fromPos, toPos;
            GridToWorld(from, fromPos);
            GridToWorld(to, toPos);
            
            XMFLOAT3 moveDir = MiMath::Normalize(MiMath::Subtract(toPos, fromPos));
            XMFLOAT3 normal = MiMath::Normalize(fromCell.normal);

            return std::abs(MiMath::Dot(normal, moveDir)) <= 0.05f;
        }
    }

    return false;
}

/// @brief World座標の始点から終点までの経路を探索する。8方向A*で経路を求める
EnemyAiWorld::PathQueryResult NavigationSystem::FindPath(
    EnemyAIWorldContext& context,
    const DirectX::XMFLOAT3& start, 
    const DirectX::XMFLOAT3& goal,
    const EnemyAiAgent::NavigationAgentSettings& agent) const
{
    // TODO: TacticalQuerySystemからの情報も使って、経路探索のコストを調整する予定

    GridCoord startCoord, goalCoord;
    if (!WorldToGrid(start, startCoord) || !WorldToGrid(goal, goalCoord)) {
        return { EnemyAiWorld::PathQueryStatus::InvalidStart, {} };
    }
    if (!IsWalkable(startCoord, agent) || !IsWalkable(goalCoord, agent)) {
        return { EnemyAiWorld::PathQueryStatus::InvalidStart, {} };
    }

    // 調査ノードを生成し、探索を開始する
    std::vector<SearchNode> nodes(m_cells.size());
    std::vector<int> openList;

    int startIndex = startCoord.x * m_buildSettings.cellCountZ + startCoord.z;
    int goalIndex = goalCoord.x * m_buildSettings.cellCountZ + goalCoord.z;

    nodes[startIndex].gCost = 0.0f;
    nodes[startIndex].parentIndex = -1;
    openList.push_back(startIndex);

    // === 探索ループ ===
    while (openList.size() > 0) {
        int currentIndex = -1;

        // 最小コストのノードを選択する
        float minFCost = std::numeric_limits<float>::infinity();
        for (int idx : openList) {
            if (nodes[idx].gCost < minFCost) {
                minFCost = nodes[idx].gCost;
                currentIndex = idx;
            }
        }

        // ゴールに到達すれば経路を復元して探索を終了する
        if (currentIndex == goalIndex) {
            EnemyAiWorld::NavigationPath path;
            path.waypoints.push_back(goal);

            // 親のインデックスを辿っていき、経路を復元する
            int pathIndex = currentIndex;
            while (pathIndex != -1) {
                GridCoord coord = { pathIndex / m_buildSettings.cellCountZ, pathIndex % m_buildSettings.cellCountZ };
                XMFLOAT3 worldPos;
                GridToWorld(coord, worldPos);
                path.waypoints.push_back(worldPos);
                pathIndex = nodes[pathIndex].parentIndex;
            }
            path.waypoints.push_back(start);

            // ゴールからの経路なので、反転
            std::reverse(path.waypoints.begin(), path.waypoints.end());
            return { EnemyAiWorld::PathQueryStatus::Success, path };
        }

        // === 探索を行う ===
        // 周囲8方向のセルを調査する（openListからは削除し、closedにする）
        openList.erase(std::remove(openList.begin(), openList.end(), currentIndex), openList.end());
        nodes[currentIndex].closed = true;

        GridCoord currentCoord{ currentIndex / m_buildSettings.cellCountZ, currentIndex % m_buildSettings.cellCountZ };
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) continue;

                GridCoord neighborCoord = { currentCoord.x + dx, currentCoord.z + dz };
                if (!ValidateCellCoord(neighborCoord)) continue;
                
                // 隣接セルをチェック
                int neighborIndex = neighborCoord.x * m_buildSettings.cellCountZ + neighborCoord.z;
                if (nodes[neighborIndex].closed) continue;
                if (!CanTraverse(currentCoord, neighborCoord, agent)) continue;
                
                // 隣接セルのコストを計算する（斜め移動は1.414倍とする）
                float tentativeGCost = nodes[currentIndex].gCost + ((dx != 0 && dz != 0) ? 1.414f : 1.0f);
                
                // コストが現在のものより小さい場合は更新する
                bool inOpenList = std::find(openList.begin(), openList.end(), neighborIndex) != openList.end();
                if (tentativeGCost < nodes[neighborIndex].gCost) {
                    nodes[neighborIndex].gCost = tentativeGCost;
                    nodes[neighborIndex].parentIndex = currentIndex;
                    if (!inOpenList) {
                        openList.push_back(neighborIndex);
                    }
                }
            }
        }
    }

    // 有効な経路が存在しなかった、という状態
    return { EnemyAiWorld::PathQueryStatus::Unreachable, {} };
}

void NavigationSystem::SmoothPath(
    const EnemyAIWorldContext& context,
    const EnemyAiAgent::NavigationAgentSettings& agent, 
    EnemyAiWorld::NavigationPath& path) const
{
    // TODO: CanMoveDirectlyで移動可能な区間の中間点だけを省く。

    // グリッドで考えたほうが良さそう？
}

#pragma endregion

/// @brief 1セルの地形を取得する
EnemyAiWorld::GridCell NavigationSystem::SampleCell(
    IScene* scene,
    const EnemyAiWorld::NavigationGridSettings& settings, 
    EnemyAiWorld::GridCoord coord) const
{
    EnemyAiWorld::GridCell cell;

    XMFLOAT2 samplePosition = { 
        settings.origin().x + (coord.x + 0.5f) * settings.cellSize,
        settings.origin().y + (coord.z + 0.5f) * settings.cellSize
    };

    // SphereCastで地面を確認（半径は0.1程度としておく）
    XMFLOAT3 rayOrigin = { samplePosition.x, settings.rayTopPosition, samplePosition.y };
    XMFLOAT3 rayDirection = { 0.0f, -1.0f, 0.0f };
    float rayLength = settings.rayTopPosition - settings.rayBottomPosition;

    RaycastHit outHit;
    if (CollisionQuery::SphereCast(scene, outHit, rayOrigin, rayDirection, 0.1f, rayLength, SAMPLE_LAYER_MASK)) {
        cell.height = outHit.hitPoint.y;
        cell.normal = outHit.hitNormal;
        if (cell.normal.y < cosf(XMConvertToRadians(settings.slopeThreshold))) {
            cell.type = CellType::SteepSlope;
        }
        else {
            cell.type = CellType::Ground;
        }
        return cell;
    }

    cell.type = EnemyAiWorld::CellType::NoGround;
    return cell;
}

/// @brief デバッグ用にグリッドを描画する
void NavigationSystem::DrawDebugGrid() const
{
    if (!m_isBuilt) return;

    for (int x = 0; x < m_buildSettings.cellCountX; x++) {
        for (int z = 0; z < m_buildSettings.cellCountZ; z++) {
            GridCoord coord{ x, z };
            const GridCell* cell = GetCell(coord);
            if (!cell) continue;
            XMFLOAT3 worldPos;
            if (!GridToWorld(coord, worldPos)) continue;

            // デバッグ描画の色をセルタイプに応じて設定
            XMFLOAT4 color;
            switch (cell->type) {
                case CellType::Ground: color = { 0.0f, 1.0f, 0.0f, 1.0f }; break; // 緑
                case CellType::NoGround: color = { 1.0f, 0.0f, 0.0f, 1.0f }; break; // 赤
                case CellType::Obstacle: color = { 1.0f, 1.0f, 0.0f, 1.0f }; break; // 黄
                case CellType::SteepSlope: color = { 1.0f, 0.5f, 0.0f, 1.0f }; break; // オレンジ
                default: color = { 1.0f, 1.0f, 1.0f, 1.0f }; break; // 白
            }

            // グリッドセルの斜め線を描画する
            float halfSize = m_buildSettings.cellSize * 0.5f;
            XMFLOAT3 minPos = { worldPos.x - halfSize, worldPos.y - halfSize * 0.1f, worldPos.z - halfSize };
            XMFLOAT3 maxPos = { worldPos.x + halfSize, worldPos.y + halfSize * 0.1f, worldPos.z + halfSize };
            DebugRenderer_DrawLine({ minPos.x, minPos.y, minPos.z }, { maxPos.x, maxPos.y, maxPos.z }, color);
        }
    }
}