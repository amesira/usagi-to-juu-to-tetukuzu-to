//---------------------------------------------------
// File  ：_/E30_Combat/Approach/enemy_approach_navigation.h
// Date  ：2026/09/10
// Author：Miu Kitamura
//
// ・PathFollowerの経路再探索を要求し、進捗を監視する
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

struct EnemyApproachContext;

class EnemyApproachNavigation {
private:
    DirectX::XMFLOAT3 m_lastPathTargetPosition = {};
    DirectX::XMFLOAT3 m_lastProgressPosition = {};

    float m_repathTimer = 0.0f;
    float m_minRepathTimer = 0.0f;
    float m_retryTimer = 0.0f;
    float m_stuckCheckTimer = 0.0f;
    
    int m_consecutivePathFailures = 0;
    int m_consecutiveStuckChecks = 0;

public:
    void Start(EnemyApproachContext& context);
    EnemyCombatStatus Update(EnemyApproachContext& context, float deltaTime);
    void Cancel(EnemyApproachContext& context);

};
