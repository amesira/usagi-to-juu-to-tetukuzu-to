#include "Game/ControllerBehavior/EnemyAI/navigation_system.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"
#include <cassert>
#include <cmath>
#include <iostream>

// 本テストは探索を人工グリッドで検証する。物理・描画の統合はゲーム側で確認。
bool CollisionQuery::SphereCast(IScene*, RaycastHit&, const XMFLOAT3&, const XMFLOAT3&,
    float, float, CollisionLayerMask) { return false; }
void DebugRenderer_DrawLine(XMFLOAT3, XMFLOAT3, XMFLOAT4) {}

struct NavigationSystemTestAccess {
    static void SetGrid(NavigationSystem& nav, int x, int z) {
        nav.ClearGrid();
        nav.m_buildSettings = {};
        nav.m_buildSettings.cellCountX=x;
        nav.m_buildSettings.cellCountZ=z;
        nav.m_buildSettings.center={x*0.5f,z*0.5f};
        nav.m_cells.assign(x*z, {EnemyAiWorld::CellType::Ground,0,{0,1,0}});
        nav.m_isBuilt=true;
    }
    static EnemyAiWorld::GridCell& Cell(NavigationSystem& nav,int x,int z) {
        return nav.m_cells[x*nav.m_buildSettings.cellCountZ+z];
    }
};

int main() {
    using namespace EnemyAiWorld;
    using Access=NavigationSystemTestAccess;
    EnemyAIWorldContext context;
    EnemyAiAgent::NavigationAgentSettings agent;
    agent.radius=0.1f;
    NavigationSystem nav;
    assert(nav.FindPath(context,{},{},agent).status==PathQueryStatus::NotReady);
    Access::SetGrid(nav,7,5);
    GridCoord coord{99,99};
    assert(!nav.WorldToGrid({-0.01f,0,1},coord) && coord.x==99);
    assert(!nav.WorldToGrid({7,0,1},coord));
    assert(nav.WorldToGrid({6.5f,0,4.5f},coord) && coord.x==6 && coord.z==4);
    Access::Cell(nav,5,2).height=0.25f;
    XMFLOAT3 position;
    assert(nav.GridToWorld({5,2},position) && position.y==0.25f);
    Access::Cell(nav,5,2).height=0;
    auto path=nav.FindPath(context,{1.5f,0,2.5f},{5.5f,0,2.5f},agent);
    assert(path.status==PathQueryStatus::Success);
    nav.SmoothPath(context,agent,path.path);
    assert(path.path.waypoints.size()==2);
    // 壁を迂回し、平滑化でも壁を突き抜けない。
    for (int z=1;z<4;++z) Access::Cell(nav,3,z).type=CellType::Obstacle;
    path=nav.FindPath(context,{1.5f,0,2.5f},{5.5f,0,2.5f},agent);
    assert(path.status==PathQueryStatus::Success);
    nav.SmoothPath(context,agent,path.path);
    assert(path.path.waypoints.size()>2);
    Access::Cell(nav,3,0).type=Access::Cell(nav,3,4).type=CellType::Obstacle;
    assert(nav.FindPath(context,{1.5f,0,2.5f},{5.5f,0,2.5f},agent).status==PathQueryStatus::Unreachable);
    assert(nav.FindPath(context,{3.5f,0,2.5f},{5.5f,0,2.5f},agent).status==PathQueryStatus::InvalidStart);
    assert(nav.FindPath(context,{1.5f,0,2.5f},{3.5f,0,2.5f},agent).status==PathQueryStatus::InvalidGoal);
    // 斜めの角抜け。
    Access::SetGrid(nav,5,5);
    Access::Cell(nav,2,1).type=CellType::Obstacle;
    assert(!nav.CanTraverse(context,{1,1},{2,2},agent));
    // 段差とAgent別の傾斜。
    Access::SetGrid(nav,5,5);
    Access::Cell(nav,2,2).height=0.8f;
    assert(!nav.CanTraverse(context,{1,2},{2,2},agent));
    agent.maxStepHeight=1;
    assert(nav.CanTraverse(context,{1,2},{2,2},agent));
    auto& slope=Access::Cell(nav,2,2);
    slope.height=0; slope.type=CellType::SteepSlope;
    slope.normal={0,0.8660254f,0.5f};
    agent.maxSlopeDegrees=20;
    assert(!nav.IsWalkable(context,{2,2},agent));
    agent.maxSlopeDegrees=45;
    assert(nav.IsWalkable(context,{2,2},agent));
    // 1セル幅の通路と半径。
    Access::SetGrid(nav,5,5);
    for(int z=0;z<5;++z) {
        Access::Cell(nav,1,z).type=CellType::Obstacle;
        Access::Cell(nav,3,z).type=CellType::Obstacle;
    }
    assert(nav.IsWalkable(context,{2,2},agent));
    agent.radius=0.6f;
    assert(!nav.IsWalkable(context,{2,2},agent));
    // 同一セル、範囲外・無効Agent、Clear後の状態。
    agent.radius=0.1f;
    path=nav.FindPath(context,{2.4f,0,2.4f},{2.6f,0,2.6f},agent);
    assert(path.status==PathQueryStatus::Success);
    assert(path.path.waypoints.front().x==2.4f && path.path.waypoints.back().x==2.6f);
    agent.radius=-1;
    assert(!nav.IsWalkable(context,{2,2},agent));
    nav.ClearGrid();
    assert(!nav.GetCell({0,0}));
    assert(!nav.BuildGrid(context));
    std::cout << "Navigation tests passed\n";
}
