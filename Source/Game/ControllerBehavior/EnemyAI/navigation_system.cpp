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

using namespace DirectX;
using namespace EnemyAiWorld;
using namespace EnemyAiAgent;

void NavigationSystem::Initialize(EnemyAIWorldContext& context)
{
    m_isBuilt = false;
    BuildGrid(context);
}

void NavigationSystem::Update(EnemyAIWorldContext& context, float deltaTime)
{
    if (!m_isBuilt) {
        BuildGrid(context);
    }

    DrawDebugGrid(context);
}

void NavigationSystem::Finalize(EnemyAIWorldContext& context)
{
    ClearGrid(context);
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
            GridCell cell = SampleCell(context, m_buildSettings, coord);
            m_cells.push_back(cell);
        }
    }
    return true;
}

#pragma region NavigationCell変換
/// @brief World座標XZをセル座標へ変換する。範囲外・未生成ならfalse
bool NavigationSystem::WorldToGrid(
    const EnemyAIWorldContext& context,
    const DirectX::XMFLOAT3& position, 
    EnemyAiWorld::GridCoord& outChoord) const
{
    if (!m_isBuilt) return false;

    outChoord.x = static_cast<int>((position.x - m_buildSettings.origin().x) / m_buildSettings.cellSize);
    outChoord.z = static_cast<int>((position.z - m_buildSettings.origin().y) / m_buildSettings.cellSize);
    if (outChoord.x < 0 || outChoord.x >= m_buildSettings.cellCountX ||
        outChoord.z < 0 || outChoord.z >= m_buildSettings.cellCountZ) {
        return false;
    }
    return true;
}

/// @brief セル中心XZと保存された地面高さYを返す。地面なし等はfalse
bool NavigationSystem::GridToWorld(
    const EnemyAIWorldContext& context,
    EnemyAiWorld::GridCoord coord, 
    DirectX::XMFLOAT3& outPosition) const
{
    if (!m_isBuilt) return false;
    if (coord.x < 0 || coord.x >= m_buildSettings.cellCountX ||
        coord.z < 0 || coord.z >= m_buildSettings.cellCountZ) {
        return false;
    }

    GridCell cell = m_cells[coord.z * m_buildSettings.cellCountX + coord.x];
    if (cell.type == CellType::NoGround || cell.type == CellType::Unknown) {
        return false;
    }

    outPosition = {
        m_buildSettings.origin().x + (coord.x + 0.5f) * m_buildSettings.cellSize,
        cell.height,
        m_buildSettings.origin().y + (coord.z + 0.5f) * m_buildSettings.cellSize
    };
    return true;
}

const EnemyAiWorld::GridCell* NavigationSystem::GetCell(
    const EnemyAIWorldContext& context,
    EnemyAiWorld::GridCoord coord) const
{
    if (!m_isBuilt) return nullptr;
    if (coord.x < 0 || coord.x >= m_buildSettings.cellCountX || 
        coord.z < 0 || coord.z >= m_buildSettings.cellCountZ) {
        return nullptr;
    }

    return &m_cells[coord.z * m_buildSettings.cellCountX + coord.x];
}
#pragma endregion

#pragma region NavigationPath探索
/// @brief このセルが歩行可能か判定する
bool NavigationSystem::IsWalkable(
    const EnemyAIWorldContext& /*context*/,
    EnemyAiWorld::GridCoord /*coord*/, 
    const EnemyAiAgent::NavigationAgentSettings& /*agent*/) const
{
    // TODO: Agent設定を検証し、地面・傾斜・半径分の余白を判定する。
    return false;
}

bool NavigationSystem::CanTraverse(
    const EnemyAIWorldContext& /*context*/,
    EnemyAiWorld::GridCoord /*from*/, EnemyAiWorld::GridCoord /*to*/,
    const EnemyAiAgent::NavigationAgentSettings& /*agent*/) const
{
    // TODO: 隣接判定、両セルの通行条件、段差、斜め移動の角抜けを確認する。
    return false;
}

EnemyAiWorld::PathQueryResult NavigationSystem::FindPath(const EnemyAIWorldContext& /*context*/,
    const DirectX::XMFLOAT3& /*start*/, const DirectX::XMFLOAT3& /*goal*/,
    const EnemyAiAgent::NavigationAgentSettings& /*agent*/) const
{
    // TODO: 始終点検証、8方向A*、親セルからの経路復元、World座標への変換。
    return {}; // 未実装のためNotReady。成功として返さない。
}

void NavigationSystem::SmoothPath(const EnemyAIWorldContext& /*context*/,
    const EnemyAiAgent::NavigationAgentSettings& /*agent*/, EnemyAiWorld::NavigationPath& /*path*/) const
{
    // TODO: CanMoveDirectlyで移動可能な区間の中間点だけを省く。
}
#pragma endregion

/// @brief 1セルの地形を取得する
EnemyAiWorld::GridCell NavigationSystem::SampleCell(
    const EnemyAIWorldContext& context,
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
    if (CollisionQuery::SphereCast(context.scene, outHit, rayOrigin, rayDirection, 0.1f, rayLength, m_sampleLayerMask)) {
        cell.type = EnemyAiWorld::CellType::Ground;
        cell.height = outHit.hitPoint.y;
        cell.normal = outHit.hitNormal;
        return cell;
    }

    cell.type = EnemyAiWorld::CellType::NoGround;
    return cell;
}

bool NavigationSystem::CanMoveDirectly(
    const EnemyAIWorldContext& /*context*/,
    const DirectX::XMFLOAT3& /*from*/, 
    const DirectX::XMFLOAT3& /*to*/,
    const EnemyAiAgent::NavigationAgentSettings& /*agent*/) const
{
    // TODO: 区間全体の地面・段差・傾斜・半径分の余白を確認する。
    return false;
}

/// @brief デバッグ用にグリッドを描画する
void NavigationSystem::DrawDebugGrid(const EnemyAIWorldContext& context) const
{
    if (!m_isBuilt) return;

    for (int x = 0; x < m_buildSettings.cellCountX; x++) {
        for (int z = 0; z < m_buildSettings.cellCountZ; z++) {
            GridCoord coord{ x, z };
            const GridCell* cell = GetCell(context, coord);
            if (!cell) continue;
            XMFLOAT3 worldPos;
            if (!GridToWorld(context, coord, worldPos)) continue;

            // デバッグ描画の色をセルタイプに応じて設定
            XMFLOAT4 color;
            switch (cell->type) {
                case CellType::Ground: color = { 0.0f, 1.0f, 0.0f, 1.0f }; break; // 緑
                case CellType::NoGround: color = { 1.0f, 0.0f, 0.0f, 1.0f }; break; // 赤
                case CellType::Obstacle: color = { 1.0f, 1.0f, 0.0f, 1.0f }; break; // 黄
                case CellType::SteepSlope: color = { 1.0f, 0.5f, 0.0f, 1.0f }; break; // オレンジ
                default: color = { 1.0f, 1.0f, 1.0f, 1.0f }; break; // 白
            }

            // グリッドセルの中心位置に小さな立方体を描画する（デバッグ用）
            float halfSize = m_buildSettings.cellSize * 0.5f;
            XMFLOAT3 minPos = { worldPos.x - halfSize, worldPos.y - halfSize * 0.1f, worldPos.z - halfSize };
            XMFLOAT3 maxPos = { worldPos.x + halfSize, worldPos.y + halfSize * 0.1f, worldPos.z + halfSize };
            DebugRenderer_DrawLine({ minPos.x, minPos.y, minPos.z }, { maxPos.x, minPos.y, minPos.z }, color);
        }
    }
}