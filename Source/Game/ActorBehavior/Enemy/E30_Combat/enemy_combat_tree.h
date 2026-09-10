//---------------------------------------------------
// File  ：_/E30_Combat/enemy_combat_tree.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵の戦闘行動を管理するBehaviorTree。登録順を優先度として、一つのCombatだけを実行する。
// ・プレイヤーのActionMachineと似ているが、こちらは排他的に制御する
//---------------------------------------------------
#pragma once
#include <vector>
#include "enemy_combat_base.h"

class EnemyContext;

/// @brief 登録順を優先度として、一つのCombatだけを実行する簡易BehaviorTree。
class EnemyCombatTree {
    std::vector<EnemyCombatBase*> m_combatBehaviors;
    EnemyCombatBase* m_activeBehavior = nullptr;

public:
    void Initialize(EnemyContext& context);
    void Finalize(EnemyContext& context);

    void RegisterBehavior(EnemyCombatBase& behavior);
    void ClearBehaviors(EnemyContext& context);

    void Update(EnemyContext& context, float deltaTime);
    void Cancel(EnemyContext& context);

    const EnemyCombatBase* GetActiveBehavior() const { return m_activeBehavior; }
};
