#include "Game/ControllerBehavior/EnemyAI/navigation_system.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"
#include <cassert>
#include <iostream>

bool CollisionQuery::SphereCast(IScene*, RaycastHit&, const XMFLOAT3&, const XMFLOAT3&, float, float, CollisionLayerMask) { return false; }
void DebugRenderer_DrawLine(XMFLOAT3, XMFLOAT3, XMFLOAT4) {}
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
    std::cout << "Navigation smoothing tests passed\n";
}
