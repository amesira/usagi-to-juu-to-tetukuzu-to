#include "enemy_combat_tree.h"

#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"

void EnemyCombatTree::Initialize(EnemyContext& context)
{
    Abort(context);
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
    Abort(context);
    m_combatBehaviors.clear();
}

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
        Abort(context);
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

void EnemyCombatTree::Abort(EnemyContext& context)
{
    if (!m_activeBehavior) return;
    m_activeBehavior->Abort(context);
    m_activeBehavior = nullptr;
}
