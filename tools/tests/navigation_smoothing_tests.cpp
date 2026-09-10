#include "Game/ControllerBehavior/EnemyAI/navigation_system.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"
#include "Game/ControllerBehavior/EnemyAI/tactical_query_system.h"
#include <cassert>
#include <cmath>
#include <iostream>

bool CollisionQuery::SphereCast(IScene*, RaycastHit&, const XMFLOAT3&, const XMFLOAT3&, float, float, CollisionLayerMask) { return false; }
void DebugRenderer_DrawLine(XMFLOAT3, XMFLOAT3, XMFLOAT4) {}
// 制御したコストマップでNavigationの積分・ID受け渡しを検証する。
static bool useDensity = false;
float TacticalQuerySystem::GetTacticalCost(const EnemyAIWorldContext&, const EnemyAiWorld::GridCoord& coord, int enemyId) {
    return useDensity && enemyId != 42 && coord.x == 4 ? 10.0f : 0.0f;
}
struct NavigationOptimizationTestAccess {
    static void Build(NavigationSystem& nav) {
        nav.ClearGrid();
        nav.m_buildSettings = {};
        nav.m_buildSettings.cellCountX = 10;
        nav.m_buildSettings.cellCountZ = 10;
        nav.m_buildSettings.cellSize = 1;
        nav.m_buildSettings.center = {5,5};
        nav.m_cells.assign(100, {EnemyAiWorld::CellType::Ground,0,{0,1,0}});
        nav.m_isBuilt = true;
    }
    static EnemyAiWorld::GridCell& Cell(NavigationSystem& nav, int x, int z) { return nav.m_cells[x*10+z]; }
};
int main() {
    using Access = NavigationOptimizationTestAccess;
    using namespace EnemyAiWorld;
    NavigationSystem nav;
    EnemyAIWorldContext context;
    EnemyAiAgent::NavigationAgentSettings agent;
    agent.radius = 0.1f;
    assert(!nav.CanMoveDirectly({2,0,2},{3,0,3},agent));
    Access::Build(nav);
    assert(nav.CanMoveDirectly({1.5f,0,1.5f},{8.5f,0,8.5f},agent));
    assert(nav.CanMoveDirectly({8.5f,0,8.5f},{1.5f,0,1.5f},agent));
    assert(nav.CanMoveDirectly({2.5f,0,2.5f},{2.5f,0,2.5f},agent));
    assert(!nav.CanMoveDirectly({-0.01f,0,2},{2,0,2},agent));
    assert(!nav.CanMoveDirectly({0.05f,0,2},{2,0,2},agent));
    Access::Cell(nav,4,4).type = CellType::Obstacle;
    assert(!nav.CanMoveDirectly({1.5f,0,1.5f},{8.5f,0,8.5f},agent));
    assert(!nav.CanMoveDirectly({3.5f,0,4},{5.5f,0,4},agent));
    assert(!nav.CanMoveDirectly({3.5f,0,3.95f},{5.5f,0,3.95f},agent));
    assert(nav.CanMoveDirectly({3.5f,0,3.7f},{5.5f,0,3.7f},agent));
    agent.radius = 0.4f;
    assert(!nav.CanMoveDirectly({3.5f,0,3.7f},{5.5f,0,3.7f},agent));
    agent.radius = 0;
    assert(!nav.CanMoveDirectly({3.5f,0,4.5f},{4.5f,0,3.5f},agent));
    agent.radius = 0.1f;
    Access::Build(nav);
    Access::Cell(nav,4,4).type = CellType::NoGround;
    assert(!nav.CanMoveDirectly({2.5f,0,4.5f},{6.5f,0,4.5f},agent));
    Access::Build(nav);
    Access::Cell(nav,4,4).height = 2;
    assert(!nav.CanMoveDirectly({2.5f,0,4.5f},{6.5f,0,4.5f},agent));
    Access::Cell(nav,4,4).height = 0.2f;
    assert(nav.CanMoveDirectly({2.5f,0,4.5f},{6.5f,0,4.5f},agent));
    Access::Cell(nav,4,4).normal = {1,0,0};
    assert(!nav.CanMoveDirectly({2.5f,0,4.5f},{6.5f,0,4.5f},agent));
    Access::Build(nav);
    NavigationPath path;
    nav.SmoothPath(context,agent,path);
    assert(path.waypoints.empty());
    path.waypoints = {{1.5f,7,1.5f},{2.5f,0,2.5f},{4.5f,9,4.5f}};
    nav.SmoothPath(context,agent,path);
    assert(path.waypoints.size()==2 && path.waypoints.front().y==7 && path.waypoints.back().y==9);
    for(int z=2;z<=7;++z) Access::Cell(nav,4,z).type = CellType::Obstacle;
    auto result = nav.FindPath(context,{2.5f,0,4.5f},{6.5f,0,4.5f},agent);
    assert(result.status==PathQueryStatus::Success);
    const auto count = result.path.waypoints.size();
    nav.SmoothPath(context,agent,result.path);
    assert(result.path.waypoints.size()>2 && result.path.waypoints.size()<count);
    for(size_t i=1;i<result.path.waypoints.size();++i)
        assert(nav.CanMoveDirectly(result.path.waypoints[i-1],result.path.waypoints[i],agent));
    Access::Build(nav);
    TacticalQuerySystem tactical;
    context.tacticalQuery = &tactical;
    useDensity = true;
    const XMFLOAT3 a{2.5f,0,4.5f}, b{6.5f,0,4.5f};
    assert(std::abs(nav.CalculateSegmentCost(context,a,b)-14.0)<1e-6);
    assert(std::abs(nav.CalculateSegmentCost(context,b,a)-14.0)<1e-6);
    assert(std::abs(nav.CalculateSegmentCost(context,a,b,42)-4.0)<1e-6);
    // セル境界の両側の最大値を使う。距離2を二重に数えない。
    assert(std::abs(nav.CalculateSegmentCost(context,{4,0,2},{4,0,4})-22.0)<1e-6);
    assert(nav.CalculateSegmentCost(context,a,a)==0);
    assert(!std::isfinite(nav.CalculateSegmentCost(context,{-1,0,2},a)));
    // 高コスト列を横切る長さを短く保つ回り道。
    path.waypoints={{2.5f,0,2.5f},{3.5f,0,6.5f},{5.5f,0,6.5f},{6.5f,0,7.5f}};
    auto original=path;
    double originalCost=0;
    for(size_t i=1;i<path.waypoints.size();++i)
        originalCost+=nav.CalculateSegmentCost(context,path.waypoints[i-1],path.waypoints[i]);
    nav.SmoothPath(context,agent,path);
    double smoothedCost=0;
    for(size_t i=1;i<path.waypoints.size();++i)
        smoothedCost+=nav.CalculateSegmentCost(context,path.waypoints[i-1],path.waypoints[i]);
    assert(path.waypoints.size()>2 && smoothedCost<=originalCost+1e-5);
    nav.SmoothPath(context,agent,original,42);
    assert(original.waypoints.size()==2);
    std::cout << "Navigation smoothing tests passed\n";
}
