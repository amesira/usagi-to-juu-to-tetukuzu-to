//===================================================
// File  ：_/EnemyAI/navigation_system.cpp
// Date  ：2026/09/09
// Author：Miu Kitamura
//===================================================
#include <queue>
#include <chrono>

#include "navigation_system.h"
#include "enemy_ai_agent_settings_asset.h"
#include "tactical_query_system.h"

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
    const EnemyAiAgent::NavigationAgentSettings& agent,
    const int enemyId) const
{
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
                const float tacticalCost = context.tacticalQuery
                    ? (std::max)(0.0f, context.tacticalQuery->GetTacticalCost(context, neighborCoord, enemyId)) : 0.0f;
                float tentativeGCost = nodes[currentIndex].gCost
                    + ((dx != 0 && dz != 0) ? 1.414f : 1.0f) * (1.0f + tacticalCost);
                
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

bool NavigationSystem::CanMoveDirectly(
    const XMFLOAT3& from, const XMFLOAT3& to,
    const NavigationAgentSettings& agent) const
{
    if (!m_isBuilt) return false;

    const double size = m_buildSettings.cellSize;
    const auto origin = m_buildSettings.origin();
    const double ax = (from.x - origin.x) / size, az = (from.z - origin.y) / size;
    const double bx = (to.x - origin.x) / size, bz = (to.z - origin.y) / size;
    const double radius = agent.radius / size;

    // WorldToGridの負数の整数切り捨てに依存せず、占有範囲ごと境界を確認する。
    if ((std::min)(ax, bx) - radius < 0 || (std::min)(az, bz) - radius < 0
        || (std::max)(ax, bx) + radius >= m_buildSettings.cellCountX
        || (std::max)(az, bz) + radius >= m_buildSettings.cellCountZ) return false;

    const double dx = bx - ax, dz = bz - az;
    std::vector<double> cuts{0.0, 1.0};
    const auto addCuts = [&](double a, double b) {
        if (a == b) return;
        for (int boundary = static_cast<int>(std::floor((std::min)(a, b))) + 1;
            boundary < (std::max)(a, b); ++boundary) {
            cuts.push_back((boundary - a) / (b - a));
        }
    };
    addCuts(ax, bx);
    addCuts(az, bz);
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    std::vector<signed char> cache(m_cells.size(), -1);

    // 境界上は両側、角上は4セルを確認する（supercover）。
    const auto cellsAt = [&](double t) {
        const double x = ax + dx * t, z = az + dz * t;
        constexpr double epsilon = 1e-9;
        std::vector<GridCoord> cells;
        for (int ix = static_cast<int>(std::floor(x - epsilon)); ix <= static_cast<int>(std::floor(x + epsilon)); ++ix) {
            for (int iz = static_cast<int>(std::floor(z - epsilon)); iz <= static_cast<int>(std::floor(z + epsilon)); ++iz) {
                cells.push_back({ix, iz});
            }
        }
        return cells;
    };
    std::vector<GridCoord> previous;
    const auto checkCenter = [&](double t) {
        auto cells = cellsAt(t);
        for (auto cell : cells) {
            if (!CachedIsWalkable(cell, agent, &cache, nullptr)) return false;
            for (auto other : cells) {
                if ((cell.x != other.x || cell.z != other.z)
                    && !CanTraverseImpl(other, cell, agent, &cache, nullptr)) return false;
            }
            for (auto other : previous) {
                if ((cell.x != other.x || cell.z != other.z)
                    && !CanTraverseImpl(other, cell, agent, &cache, nullptr)) return false;
            }
        }
        previous = std::move(cells);
        return true;
    };

    if (!checkCenter(0.0)) return false;
    
    const float minUpDot = std::cos(XMConvertToRadians(agent.maxSlopeDegrees));
    for (size_t i = 1; i < cuts.size(); ++i) {
        const double t0 = cuts[i - 1], t1 = cuts[i], mid = (t0 + t1) * 0.5;
        if (!checkCenter(mid)) return false;

        const auto* center = GetCell({static_cast<int>(std::floor(ax + dx * mid)),
            static_cast<int>(std::floor(az + dz * mid))});
        const double x0 = ax + dx * t0, z0 = az + dz * t0;
        const double x1 = ax + dx * t1, z1 = az + dz * t1;

        // 円を内包する正方形の掃引で保守的に判定する。角付近は余裕を多めに取る。
        for (int x = static_cast<int>(std::floor((std::min)(x0, x1) - radius));
            x <= static_cast<int>(std::floor((std::max)(x0, x1) + radius)); ++x) {
            for (int z = static_cast<int>(std::floor((std::min)(z0, z1) - radius));
                z <= static_cast<int>(std::floor((std::max)(z0, z1) + radius)); ++z) {
                double enter = 0.0, leave = 1.0;
                const auto clipAxis = [&](double a, double delta, double low, double high) {
                    if (delta == 0.0) return a >= low && a <= high;
                    double first = (low - a) / delta, last = (high - a) / delta;
                    if (first > last) std::swap(first, last);
                    enter = (std::max)(enter, first);
                    leave = (std::min)(leave, last);
                    return enter <= leave;
                };
                if (!clipAxis(x0, x1 - x0, x - radius, x + 1.0 + radius)
                    || !clipAxis(z0, z1 - z0, z - radius, z + 1.0 + radius)) continue;
                const auto* cell = GetCell({x, z});
                if (!cell || cell->type == CellType::Unknown || cell->type == CellType::NoGround
                    || cell->type == CellType::Obstacle || cell->normal.y < minUpDot
                    || std::abs(cell->height - center->height) > agent.maxStepHeight) return false;
            }
        }
        if (!checkCenter(t1)) return false;
    }
    return true;
}

double NavigationSystem::CalculateSegmentCost(const EnemyAIWorldContext& context,
    const XMFLOAT3& from, const XMFLOAT3& to, int enemyId) const
{
    const double invalid = std::numeric_limits<double>::infinity();
    if (!m_isBuilt || !std::isfinite(m_buildSettings.cellSize) || m_buildSettings.cellSize <= 0) return invalid;
    for (float value : {from.x, from.y, from.z, to.x, to.y, to.z}) {
        if (!std::isfinite(value)) return invalid;
    }
    const auto origin = m_buildSettings.origin();
    const double size = m_buildSettings.cellSize;
    const double ax = (from.x - origin.x) / size, az = (from.z - origin.y) / size;
    const double bx = (to.x - origin.x) / size, bz = (to.z - origin.y) / size;
    if ((std::min)(ax, bx) < 0 || (std::min)(az, bz) < 0
        || (std::max)(ax, bx) >= m_buildSettings.cellCountX
        || (std::max)(az, bz) >= m_buildSettings.cellCountZ) return invalid;

    const double dx = bx - ax, dz = bz - az;
    const double length = std::hypot(dx, dz) * size;
    if (length == 0) return 0;
    std::vector<double> cuts{0.0, 1.0};
    const auto addCuts = [&](double a, double b) {
        if (a == b) return;
        for (int boundary = static_cast<int>(std::floor((std::min)(a, b))) + 1;
            boundary < (std::max)(a, b); ++boundary) {
            cuts.push_back((boundary - a) / (b - a));
        }
    };
    addCuts(ax, bx);
    addCuts(az, bz);
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    double cost = 0;
    for (size_t i = 1; i < cuts.size(); ++i) {
        const double mid = (cuts[i - 1] + cuts[i]) * 0.5;
        const double x = ax + dx * mid, z = az + dz * mid;
        double tacticalCost = 0;
        // 境界上を進む区間は両側の最大値。距離を二重計上しない。
        constexpr double epsilon = 1e-9;
        const int minX = static_cast<int>(std::floor(x - (dx == 0 ? epsilon : 0)));
        const int minZ = static_cast<int>(std::floor(z - (dz == 0 ? epsilon : 0)));
        for (int ix = minX; ix <= static_cast<int>(std::floor(x)); ++ix) {
            for (int iz = minZ; iz <= static_cast<int>(std::floor(z)); ++iz) {
                GridCoord coord{ix, iz};
                if (!ValidateCellCoord(coord)) return invalid;
                const double value = context.tacticalQuery
                    ? context.tacticalQuery->GetTacticalCost(context, coord, enemyId) : 0.0;
                if (!std::isfinite(value)) return invalid;
                tacticalCost = (std::max)(tacticalCost, value);
            }
        }
        cost += length * (cuts[i] - cuts[i - 1]) * (1.0 + tacticalCost);
    }
    return cost;
}

void NavigationSystem::SmoothPath(
    const EnemyAIWorldContext& context,
    const EnemyAiAgent::NavigationAgentSettings& agent, 
    EnemyAiWorld::NavigationPath& path, int enemyId) const
{
    if (!m_isBuilt || path.waypoints.size() <= 2) return;

    // 同じマップ・敵ID・区間評価で元経路と短縮候補を比較する。
    std::vector<double> prefixCost(path.waypoints.size(), 0.0);
    for (size_t i = 1; i < path.waypoints.size(); ++i) {
        const double cost = CalculateSegmentCost(context, path.waypoints[i - 1], path.waypoints[i], enemyId);
        if (!std::isfinite(cost)) return;
        prefixCost[i] = prefixCost[i - 1] + cost;
    }

    std::vector<XMFLOAT3> smoothed;
    smoothed.reserve(path.waypoints.size());
    size_t anchor = 0;
    smoothed.push_back(path.waypoints.front());
    while (anchor + 1 < path.waypoints.size()) {
        size_t next = path.waypoints.size() - 1;
        while (next > anchor + 1) {
            if (CanMoveDirectly(path.waypoints[anchor], path.waypoints[next], agent)) {
                const double shortcutCost = CalculateSegmentCost(context, path.waypoints[anchor], path.waypoints[next], enemyId);
                const double originalCost = prefixCost[next] - prefixCost[anchor];
                const double epsilon = 1e-6 * (std::max)(1.0, originalCost);
                if (std::isfinite(shortcutCost) && shortcutCost <= originalCost + epsilon) break;
            }
            --next;
        }
        // 短縮できない場合は元の隣接区間を保持する。無効な経路の修復は行わない。
        smoothed.push_back(path.waypoints[next]);
        anchor = next;
    }
    path.waypoints = std::move(smoothed);
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
