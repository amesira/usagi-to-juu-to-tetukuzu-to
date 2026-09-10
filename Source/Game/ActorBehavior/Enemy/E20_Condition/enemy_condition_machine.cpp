//===================================================
// File  ：_/E20_Condition/enemy_condition_machine.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_condition_machine.h"

#include <algorithm>

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Base/health_behavior.h"

void EnemyConditionMachine::Initialize(EnemyContext& context)
{
    m_stunRemainingTime = 0.0f;
    m_currentCondition = EvaluateCondition(context);

    EnterCondition(context, m_currentCondition);
}

void EnemyConditionMachine::Update(EnemyContext& context, float deltaTime)
{
    m_stunRemainingTime -= deltaTime;
    if (m_stunRemainingTime < 0.0f) m_stunRemainingTime = 0.0f;

    // 状態の評価と遷移
    ChangeCondition(context, EvaluateCondition(context));
}

void EnemyConditionMachine::Finalize(EnemyContext& context)
{
    ExitCondition(context, m_currentCondition);
    m_stunRemainingTime = 0.0f;
    m_currentCondition = EnemyCondition::Idle;
}

#pragma region 状態変化
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
#pragma endregion

/// @brief 現在のEnemyContextの状態から、次のEnemyConditionを評価する
EnemyCondition EnemyConditionMachine::EvaluateCondition(const EnemyContext& context) const
{
    if (context.health && context.health->IsDead()) {
        return EnemyCondition::Dead;
    }
    if (m_stunRemainingTime > 0.0f) {
        return EnemyCondition::Stun;
    }
    if (context.runtimeState.hasCombatTarget) {
        return EnemyCondition::Combat;
    }
    return EnemyCondition::Idle;
}

/// @brief 状態に入った
void EnemyConditionMachine::EnterCondition(EnemyContext&, EnemyCondition)
{
    
}

/// @brief 状態から出た
void EnemyConditionMachine::ExitCondition(EnemyContext&, EnemyCondition)
{

}
