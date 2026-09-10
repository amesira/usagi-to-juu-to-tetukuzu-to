//---------------------------------------------------
// File  ：_/E30_Combat/Approach/enemy_approach_navigation.h
// Date  ：2026/09/10
// Author：Miu Kitamura
//
// ・PathFollowerの経路再探索を要求し、進捗を監視する
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

struct EnemyApproachContext;

class EnemyApproachNavigation {
public:
    void Start(EnemyApproachContext& context);
    EnemyCombatStatus Update(EnemyApproachContext& context, float deltaTime);
    void Cancel(EnemyApproachContext& context);

};
