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
#include "enemy_ai_world_context.h"

// 共有情報と経路情報を使い、移動先の候補を評価する。
class TacticalQuerySystem {
private:
    static constexpr float UPDATE_INTERVAL = 0.2f; // タクティカルセル情報の更新間隔（秒）

    static constexpr float ENEMY_DENSITY_COST_WEIGHT = 1.0f; // 敵の密度コストの重み
    static constexpr float RESERVATION_COST_WEIGHT = 1.0f;    // 予約コストの重み

    std::vector<EnemyAiWorld::TacticalCellInfo> m_tacticalCells;
    float m_updateTimer = 0.0f;

public:
    void Initialize(const EnemyAIWorldContext& context);
    void Update(EnemyAIWorldContext& context, float deltaTime);
    void Finalize(const EnemyAIWorldContext& context);

    float GetTacticalCost(const EnemyAIWorldContext& context, const EnemyAiWorld::GridCoord& coord);

};

#endif // TACTICAL_QUERY_SYSTEM_H
