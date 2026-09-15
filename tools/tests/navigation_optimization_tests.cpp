#include "Game/ControllerBehavior/EnemyAI/navigation_system.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>

bool CollisionQuery::SphereCast(IScene*, RaycastHit&, const XMFLOAT3&, const XMFLOAT3&,
    float, float, CollisionLayerMask) { return false; }
void DebugRenderer_DrawLine(XMFLOAT3, XMFLOAT3, XMFLOAT4) {}
struct NavigationOptimizationTestAccess {
    static void Build(NavigationSystem& nav, int width, int depth, std::mt19937& rng, bool obstacles) {
        nav.ClearGrid();
        nav.m_buildSettings = {};
        nav.m_buildSettings.cellCountX=width;
        nav.m_buildSettings.cellCountZ=depth;
        nav.m_buildSettings.cellSize=1;
        nav.m_buildSettings.center={width/2.0f,depth/2.0f};
        nav.m_cells.resize(width*depth);
        for (auto& cell:nav.m_cells) {
            cell.type=obstacles && rng()%5==0 ? EnemyAiWorld::CellType::Obstacle : EnemyAiWorld::CellType::Ground;
            cell.height=0;
            cell.normal={0,1,0};
        }
        nav.m_isBuilt=true;
    }
};

// ヒューリスティック・キャッシュなしのDijkstraを比較用に使う。
float ReferenceCost(const NavigationSystem& nav,int width,int depth,
    const EnemyAiAgent::NavigationAgentSettings& agent) {
    using EnemyAiWorld::GridCoord;
    std::vector<float> costs(width*depth,INFINITY);
    std::vector<bool> closed(width*depth,false);
    const int source=depth+1,target=(width-2)*depth+depth-2;
    if (!nav.IsWalkable({1,1},agent) || !nav.IsWalkable({width-2,depth-2},agent)) return INFINITY;
    costs[source]=0;
    for (;;) {
        int best=-1;
        for(int i=0;i<width*depth;++i)
            if(!closed[i] && std::isfinite(costs[i]) && (best<0 || costs[i]<costs[best])) best=i;
        if(best<0) return INFINITY;
        if(best==target) return costs[best];
        closed[best]=true;
        GridCoord c{best/depth,best%depth};
        for(int dx=-1;dx<=1;++dx) for(int dz=-1;dz<=1;++dz) {
            GridCoord n{c.x+dx,c.z+dz};
            if(!nav.CanTraverse(c,n,agent)) continue;
            const int index=n.x*depth+n.z;
            costs[index]=(std::min)(costs[index],costs[best]+(dx && dz ? 1.414f:1.0f));
        }
    }
}

int main() {
    NavigationSystem nav;
    EnemyAIWorldContext context;
    EnemyAiAgent::NavigationAgentSettings agent;
    std::mt19937 rng(731);
    for (int test=0;test<40;++test) {
        const int width=13,depth=9;
        NavigationOptimizationTestAccess::Build(nav,width,depth,rng,test!=0);
        agent.radius=test%2 ? 0.6f:0.1f;
        const float expected=ReferenceCost(nav,width,depth,agent);
        const auto result=nav.FindPath(context,{1.5f,0,1.5f},{width-1.5f,0,depth-1.5f},agent);
        const bool success=result.status==EnemyAiWorld::PathQueryStatus::Success;
        assert(success==std::isfinite(expected));
        assert(nav.GetLastSearchStats().walkableEvaluations<=width*depth);
        if(success) {
            float cost=0;
            // start/goalの追加点を除いたセル中心の区間。
            for(size_t i=2;i+1<result.path.waypoints.size();++i) {
                EnemyAiWorld::GridCoord a,b;
                assert(nav.WorldToGrid(result.path.waypoints[i-1],a));
                assert(nav.WorldToGrid(result.path.waypoints[i],b));
                assert(nav.CanTraverse(a,b,agent));
                cost+=(a.x!=b.x && a.z!=b.z)?1.414f:1.0f;
            }
            assert(std::abs(cost-expected)<0.001f);
        }
    }
    NavigationOptimizationTestAccess::Build(nav,100,100,rng,false);
    agent.radius=3;
    assert(nav.FindPath(context,{10.5f,0,10.5f},{80.5f,0,80.5f},agent).status==EnemyAiWorld::PathQueryStatus::Success);
    const auto stats=nav.GetLastSearchStats();
    assert(stats.expandedNodes<200);
    assert(stats.walkableCacheHits>0);
    std::cout << "Passed: 40 Dijkstra comparisons. 100x100: " << stats.milliseconds
        << " ms, expanded=" << stats.expandedNodes << ", evaluations=" << stats.walkableEvaluations
        << ", cache hits=" << stats.walkableCacheHits << '\n';
}
