//---------------------------------------------------
// File  ：_/E30_Combat/enemy_combat_tree.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵の戦闘行動を管理するBehaviorTree。継続条件、割り込み可否、優先度で選択する。
// ・プレイヤーのActionMachineと似ているが、こちらは排他的に制御する
//---------------------------------------------------
#pragma once
#include <vector>
#include "enemy_combat_base.h"

class EnemyContext;

/// @brief 数値の優先度で一つのCombatを実行する。同優先度の開始候補は登録順。
class EnemyCombatTree {
    std::vector<EnemyCombatBase*> m_combatBehaviors;
    EnemyCombatBase* m_activeBehavior = nullptr;

    class EnemyAttackCombat* m_attackCombat = nullptr;

public:
    void Initialize(EnemyContext& context);
    void Finalize(EnemyContext& context);

    void RegisterBehavior(EnemyAttackCombat& behavior);
    void RegisterBehavior(EnemyCombatBase& behavior);
    void ClearBehaviors(EnemyContext& context);

    void Update(EnemyContext& context, float deltaTime);
    /// @brief Stun・Deadなどによる強制終了。割り込み可否に関係なくキャンセルする。
    void Cancel(EnemyContext& context);

    const EnemyCombatBase* GetActiveBehavior() const { return m_activeBehavior; }
    bool IsAttackReady() const;
    const EnemyAttackCombat* GetAttackCombat() const { return m_attackCombat; }
};
