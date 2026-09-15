//---------------------------------------------------
// File  ：_/RePosition/enemy_re_position_navigation.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・PathFollowerの経路再探索を要求し、進捗を監視する
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

class EnemyPathFollower;

/// @brief 固定の調整先への経路を管理する。動的な目的地選択はCombatの外で行う。
class EnemyRePositionNavigation {
    EnemyPathFollower* m_pathFollower = nullptr;
    DirectX::XMFLOAT3 m_destination = {};
    bool m_hasPath = false;

public:
    void Start(const EnemyContext& context, const DirectX::XMFLOAT3& destination);
    EnemyCombatStatus Update(const EnemyContext& context, float arrivalDistance);
    void Cancel();
};
