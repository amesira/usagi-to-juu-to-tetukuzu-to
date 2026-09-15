//---------------------------------------------------
// File  ：_/EnemyAI/tactical_query_system.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・共有情報と経路情報を使い、移動先の候補を評価する。
//---------------------------------------------------
#ifndef TACTICAL_QUERY_SYSTEM_H
#define TACTICAL_QUERY_SYSTEM_H
#include <vector>
#include <unordered_map>
#include "enemy_ai_world_context.h"

// 共有情報と経路情報を使い、移動先の候補を評価する。
class TacticalQuerySystem {
private:
    std::vector<EnemyAiWorld::TacticalCellInfo> m_tacticalCells;
    float m_updateTimer = 0.0f;

    // 影響度のある密度源（敵や障害物など）を管理するための構造体
    struct DensitySource {
        DirectX::XMFLOAT3 position;
        float influenceRadius;
        float strength;
    };
    std::unordered_map<unsigned int, DensitySource> m_densitySources;

public:
    void Initialize(const EnemyAIWorldContext& context);
    void Update(EnemyAIWorldContext& context, float deltaTime);
    void Finalize(const EnemyAIWorldContext& context);

    float GetTacticalCost(const EnemyAIWorldContext& context, const EnemyAiWorld::GridCoord& coord, int enemyId = -1);

};

#endif // TACTICAL_QUERY_SYSTEM_H
