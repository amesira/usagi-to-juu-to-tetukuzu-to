//---------------------------------------------------
// File  ：_/EnemyAI/tactical_query_system.h
// Date  ：2026/09/09
// Author：Miu Kitamura
// 
// ・共有情報と経路情報を使い、移動先の候補を評価する。
//---------------------------------------------------
#ifndef TACTICAL_QUERY_SYSTEM_H
#define TACTICAL_QUERY_SYSTEM_H
#include "enemy_ai_world_context.h"

// 共有情報と経路情報を使い、移動先の候補を評価する。
class TacticalQuerySystem {
public:
    void Initialize(const EnemyAIWorldContext& context);
    void Update(EnemyAIWorldContext& context, float deltaTime);
    void Finalize(const EnemyAIWorldContext& context);
};

#endif // TACTICAL_QUERY_SYSTEM_H
