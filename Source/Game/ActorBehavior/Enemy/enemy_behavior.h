//---------------------------------------------------
// File  ：_/Enemy/enemy_behavior.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵全体を統括するBehavior。状態判断、戦闘、移動、アニメーションを更新する。
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move.h"
#include "Game/ActorBehavior/Enemy/E20_Condition/enemy_condition_machine.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_tree.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Approach/enemy_approach_combat.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_combat.h"

#include "enemy_animation_controller.h"

class EnemyAiAgentSettingsAsset;

/// @brief 敵個体の状態判断、戦闘、移動、アニメーションを更新する司令塔。
class EnemyBehavior : public BehaviorComponent {
    EnemyContext m_context;

    EnemyLocomotionController m_locomotionController;
    EnemyMove m_moveBehavior;
    EnemyConditionMachine m_conditionMachine;
    EnemyCombatTree m_combatTree;
    EnemyAnimationController m_animationController;

    EnemyAttackCombat m_attackCombat;
    EnemyApproachCombat m_approachCombat;

    const EnemyAiAgentSettingsAsset* m_aiAgentSettings = nullptr;

public:
    EnemyBehavior() = default;
    ~EnemyBehavior() override = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // === SettingsAssetの設定 ===
    void SetupAiAgentSettings(const EnemyAiAgentSettingsAsset* settings) {
        m_aiAgentSettings = settings;
    }

    // === Enemyへのアクセス ===
    void StartStun(float duration) { m_conditionMachine.StartStun(duration); }
    EnemyCondition GetCondition() const { return m_conditionMachine.GetCurrentCondition(); }
    EnemyContext& GetContext() { return m_context; }
    const EnemyContext& GetContext() const { return m_context; }

private:
    void UpdateTargetState();

};
