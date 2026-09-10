#include <queue>
#include <chrono>
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
    if (!m_isBuilt) return false;
    if (!ValidateCellCoord(coord)) return false;

    GridCell cell = m_cells[coord.x * m_buildSettings.cellCountZ + coord.z];
    if (cell.type == CellType::Unknown || cell.type == CellType::NoGround || cell.type == CellType::Obstacle) {
        return false;
    }

    float minUpDot = std::cos(XMConvertToRadians(agent.maxSlopeDegrees));

    // 敵の半径に応じて、隣接セルまでの範囲を確認する
    int range = static_cast<int>(std::ceil(agent.radius / m_buildSettings.cellSize));
    float cellSize = m_buildSettings.cellSize;
    float halfSize = cellSize * 0.5f;

    for (int dx = -range; dx <= range; dx++) {
        for (int dz = -range; dz <= range; dz++) {
            // 敵の中心から、調査対象セルの矩形までの最短距離
            float nearestX = (std::max)(std::abs(dx) * cellSize - halfSize, 0.0f);
            float nearestZ = (std::max)(std::abs(dz) * cellSize - halfSize, 0.0f);

            // 中心セルは必ず確認。それ以外は円と重ならなければ対象外
            if ((dx != 0 || dz != 0) && 
                nearestX * nearestX + nearestZ * nearestZ >= agent.radius * agent.radius) {
                continue;
            }

            GridCoord neighborCoord{ coord.x + dx, coord.z + dz };
            if (!ValidateCellCoord(neighborCoord)) return false;

            GridCell neighborCell = m_cells[neighborCoord.x * m_buildSettings.cellCountZ + neighborCoord.z];
            if (neighborCell.type == CellType::Unknown || neighborCell.type == CellType::NoGround || neighborCell.type == CellType::Obstacle) {
                return false; // 隣接セルに地面なしまたは障害物がある場合、歩行不可
            }
            
            // 段差が許容範囲内か、斜面の角度が許容範囲内かを確認する
            bool heightAllowed = std::abs(neighborCell.height - cell.height) <= agent.maxStepHeight;
            bool slopeAllowed = neighborCell.normal.y >= minUpDot;
            if (!heightAllowed || !slopeAllowed) {
                return false;
            }
        }
    }

    return true;
}

/// @brief IsWalkableの結果をキャッシュして返す。キャッシュがあれば、同じセルの判定は1回だけ行う
/// -1 : 未評価, 0 : 歩行不可, 1 : 歩行可能
bool NavigationSystem::CachedIsWalkable(GridCoord coord,
    const EnemyAiAgent::NavigationAgentSettings& agent,
    std::vector<signed char>* cache, SearchStats* stats) const
{
    if (!m_isBuilt || !ValidateCellCoord(coord)) return false;

    // 未評価出ないなら結果を返す
    const int index = coord.x * m_buildSettings.cellCountZ + coord.z;
    if (cache && (*cache)[index] != -1) {
        if (stats) ++stats->walkableCacheHits;
        return (*cache)[index] != 0;
    }

    // 評価して結果をキャッシュする
    if (stats) stats->walkableEvaluations++;
    const bool result = IsWalkable(coord, agent);
    if (cache) {
        (*cache)[index] = result ? 1 : 0;
    }

    return result;
}

bool NavigationSystem::CanTraverseImpl(
    EnemyAiWorld::GridCoord from, 
    EnemyAiWorld::GridCoord to,
    const EnemyAiAgent::NavigationAgentSettings& agent, 
    std::vector<signed char>* cache, SearchStats* stats) const
{
    if (!m_isBuilt) return false;
    if (!ValidateCellCoord(from) || !ValidateCellCoord(to)) return false;

    int dx = to.x - from.x;
    int dz = to.z - from.z;
    if (abs(dx) > 1 || abs(dz) > 1 || (dx == 0 && dz == 0)) {
        return false; // 隣接セルではない
    }
    if (!CachedIsWalkable(from, agent, cache, stats) || !CachedIsWalkable(to, agent, cache, stats)) {
        return false; // どちらかのセルが歩行不可
    }

    GridCell fromCell = m_cells[from.x * m_buildSettings.cellCountZ + from.z];
    GridCell toCell = m_cells[to.x * m_buildSettings.cellCountZ + to.z];
    
    // === 斜め移動の場合 ===
    if (dx != 0 && dz != 0) {
        GridCoord adj1 = { from.x + dx, from.z };
        GridCoord adj2 = { from.x, from.z + dz };

        // 直線的に隣接する2つのセルを、再帰的に確認する
        return CanTraverseImpl(from, adj1, agent, cache, stats)
            && CanTraverseImpl(from, adj2, agent, cache, stats)
            && CanTraverseImpl(adj1, to, agent, cache, stats)
            && CanTraverseImpl(adj2, to, agent, cache, stats);
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

    // 計測とキャッシュは1回の探索単位。地形・Agent変更後へ持ち越さない
    m_lastSearchStats = {};
    struct SearchTimer {
        SearchStats& stats;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        ~SearchTimer() {
            stats.milliseconds = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - begin).count();
        }
    } timer{m_lastSearchStats};
    std::vector<signed char> walkableCache(m_cells.size(), -1);

    GridCoord startCoord, goalCoord;
    if (!WorldToGrid(start, startCoord) || !WorldToGrid(goal, goalCoord)) {
        return { EnemyAiWorld::PathQueryStatus::InvalidStart, {} };
    }
    if (!CachedIsWalkable(startCoord, agent, &walkableCache, &m_lastSearchStats) || !CachedIsWalkable(goalCoord, agent, &walkableCache, &m_lastSearchStats)) {
        return { EnemyAiWorld::PathQueryStatus::InvalidStart, {} };
    }

    // 調査ノードを生成し、探索を開始する
    std::vector<SearchNode> nodes(m_cells.size());

    // コスト改善時は再登録し、古い候補は取り出し時に破棄する
    std::priority_queue<OpenNode> openList;

    // ゴールまでの推定コストを計算する。斜め移動は1.414倍とする
    // この値が小さい→ゴールに近いと判断される
    // （gCost + hCost）が小さい順に探索される
    const auto estimateCost = [&](GridCoord coord) {
        const int dx = std::abs(coord.x - goalCoord.x);
        const int dz = std::abs(coord.z - goalCoord.z);
        return static_cast<float>((std::max)(dx, dz))
            + 0.414f * static_cast<float>((std::min)(dx, dz));
    };

    int startIndex = startCoord.x * m_buildSettings.cellCountZ + startCoord.z;
    int goalIndex = goalCoord.x * m_buildSettings.cellCountZ + goalCoord.z;

    nodes[startIndex].gCost = 0.0f;
    nodes[startIndex].parentIndex = -1;
    openList.push({startIndex, 0.0f, estimateCost(startCoord)});

    // === 探索ループ ===
    while (openList.size() > 0) {
        const OpenNode entry = openList.top();
        openList.pop();
        const int currentIndex = entry.index;
        if (nodes[currentIndex].closed || entry.gCost > nodes[currentIndex].gCost) continue;
        m_lastSearchStats.expandedNodes++;

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
                if (!CanTraverseImpl(currentCoord, neighborCoord, agent, &walkableCache, &m_lastSearchStats)) continue;
                
                // 隣接セルのコストを計算する（斜め移動は1.414倍とする）
                float tentativeGCost = nodes[currentIndex].gCost + ((dx != 0 && dz != 0) ? 1.414f : 1.0f);
                
                // コストが現在のものより小さい場合は更新する
                if (tentativeGCost < nodes[neighborIndex].gCost) {
                    nodes[neighborIndex].gCost = tentativeGCost;
                    nodes[neighborIndex].parentIndex = currentIndex;
                    openList.push({neighborIndex, tentativeGCost, estimateCost(neighborCoord)});
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