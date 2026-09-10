//---------------------------------------------------
// File  ：_/E20_Condition/enemy_condition_machine.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵全体のステートマシン
//---------------------------------------------------
#pragma once

class EnemyContext;

enum class EnemyCondition {
    Idle,
    Combat,
    Stun,
    Dead,
};

/// @brief 敵全体の排他的な状態を管理する。
class EnemyConditionMachine {
    EnemyCondition m_currentCondition = EnemyCondition::Idle;
    float m_stunRemainingTime = 0.0f;

public:
    void Initialize(EnemyContext& context);
    void Update(EnemyContext& context, float deltaTime);
    void Finalize(EnemyContext& context);

    void StartStun(float duration);
    void ChangeCondition(EnemyContext& context, EnemyCondition nextCondition);

    EnemyCondition GetCurrentCondition() const { return m_currentCondition; }
    bool IsCombat() const { return m_currentCondition == EnemyCondition::Combat; }

private:
    EnemyCondition EvaluateCondition(const EnemyContext& context) const;
    void EnterCondition(EnemyContext& context, EnemyCondition condition);
    void ExitCondition(EnemyContext& context, EnemyCondition condition);

};
