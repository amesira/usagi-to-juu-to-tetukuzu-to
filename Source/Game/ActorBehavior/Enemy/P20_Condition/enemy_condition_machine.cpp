#include "enemy_condition_machine.h"

#include <algorithm>

#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"
#include "Game/ActorBehavior/Base/health_behavior.h"

void EnemyConditionMachine::Initialize(EnemyContext& context)
{
    m_stunRemainingTime = 0.0f;
    m_currentCondition = EvaluateCondition(context);
    EnterCondition(context, m_currentCondition);
}

void EnemyConditionMachine::Update(EnemyContext& context, float deltaTime)
{
    m_stunRemainingTime = (std::max)(0.0f, m_stunRemainingTime - deltaTime);
    ChangeCondition(context, EvaluateCondition(context));
}

void EnemyConditionMachine::Finalize(EnemyContext& context)
{
    ExitCondition(context, m_currentCondition);
    m_stunRemainingTime = 0.0f;
    m_currentCondition = EnemyCondition::Idle;
}

void EnemyConditionMachine::StartStun(float duration)
{
    m_stunRemainingTime = (std::max)(m_stunRemainingTime, duration);
}

void EnemyConditionMachine::ChangeCondition(EnemyContext& context, EnemyCondition nextCondition)
{
    if (nextCondition == m_currentCondition) return;
    ExitCondition(context, m_currentCondition);
    m_currentCondition = nextCondition;
    EnterCondition(context, m_currentCondition);
}

EnemyCondition EnemyConditionMachine::EvaluateCondition(const EnemyContext& context) const
{
    if (context.health && context.health->IsDead()) return EnemyCondition::Dead;
    if (m_stunRemainingTime > 0.0f) return EnemyCondition::Stun;
    if (context.runtimeState.hasCombatTarget) return EnemyCondition::Combat;
    return EnemyCondition::Idle;
}

void EnemyConditionMachine::EnterCondition(EnemyContext&, EnemyCondition)
{
    // 状態固有のアニメーションや演出は、仕様確定後にここから要求する。
}

void EnemyConditionMachine::ExitCondition(EnemyContext&, EnemyCondition)
{
}
