//===================================================
// File  ：_/E30_Combat/enemy_combat_tree.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_combat_tree.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"

void EnemyCombatTree::Initialize(EnemyContext& context)
{
    Cancel(context);
}

void EnemyCombatTree::Finalize(EnemyContext& context)
{
    ClearBehaviors(context);
}

void EnemyCombatTree::RegisterBehavior(EnemyCombatBase& behavior)
{
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
    // 登録順で最初に開始条件を満たすものを選ぶ。
    EnemyCombatBase* selected = nullptr;
    for (EnemyCombatBase* behavior : m_combatBehaviors) {
        if (behavior && behavior->CanStart(context)) {
            selected = behavior;
            break;
        }
    }

    if (selected != m_activeBehavior) {
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
