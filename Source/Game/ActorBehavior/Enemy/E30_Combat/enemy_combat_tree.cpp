//===================================================
// File  ：_/E30_Combat/enemy_combat_tree.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_combat_tree.h"
#include <algorithm>

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Attack/enemy_attack_combat.h"

void EnemyCombatTree::Initialize(EnemyContext& context)
{
    Cancel(context);
}

void EnemyCombatTree::Finalize(EnemyContext& context)
{
    ClearBehaviors(context);
}

void EnemyCombatTree::RegisterBehavior(EnemyAttackCombat& behavior)
{
    m_attackCombat = &behavior;
    RegisterBehavior(static_cast<EnemyCombatBase&>(behavior));
}

void EnemyCombatTree::RegisterBehavior(EnemyCombatBase& behavior)
{
    if (std::find(m_combatBehaviors.begin(), m_combatBehaviors.end(), &behavior)
        != m_combatBehaviors.end()) return;
    m_combatBehaviors.push_back(&behavior);
}

void EnemyCombatTree::ClearBehaviors(EnemyContext& context)
{
    Cancel(context);
    m_combatBehaviors.clear();
}

/// @brief 現在のCombat行動を更新する
void EnemyCombatTree::Update(EnemyContext& context, float deltaTime)
{
    // === RuntimeState更新 ===



    for (auto* behavior : m_combatBehaviors) {
        if (behavior) behavior->UpdateBackground(context, deltaTime);
    }
    // 継続条件は開始条件と独立し、割り込み禁止より優先する。
    if (m_activeBehavior && !m_activeBehavior->CanContinue(context)) {
        Cancel(context);
    }

    // 割り込み可能な場合だけ、新しい行動の開始条件を評価する。
    // 同優先度では実行中の行動を維持し、非実行中なら登録順を使う。
    EnemyCombatBase* selected = nullptr;
    if (!m_activeBehavior || m_activeBehavior->IsInterruptible(context)) {
        for (EnemyCombatBase* behavior : m_combatBehaviors) {
            if (!behavior || behavior == m_activeBehavior) continue;
            if (m_activeBehavior && behavior->GetPriority() <= m_activeBehavior->GetPriority()) continue;
            if (selected && behavior->GetPriority() <= selected->GetPriority()) continue;
            if (behavior->CanStart(context)) selected = behavior;
        }
    }

    if (selected) {
        Cancel(context);
        m_activeBehavior = selected;
        if (m_activeBehavior) m_activeBehavior->Start(context);
    }

    if (!m_activeBehavior) return;
    const EnemyCombatStatus status = m_activeBehavior->Update(context, deltaTime);
    if (status != EnemyCombatStatus::Running) {
        m_activeBehavior->Finish(context);
        m_activeBehavior = nullptr;
    }
}

void EnemyCombatTree::Cancel(EnemyContext& context)
{
    if (!m_activeBehavior) return;
    m_activeBehavior->Cancel(context);
    m_activeBehavior = nullptr;
}
