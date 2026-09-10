#pragma once

#include "Engine/Component/behavior_component.h"
#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/P10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/P10_Locomotion/enemy_move_behavior.h"
#include "Game/ActorBehavior/Enemy/P20_Condition/enemy_condition_machine.h"
#include "Game/ActorBehavior/Enemy/P30_Combat/enemy_combat_tree.h"
#include "Game/ActorBehavior/Enemy/P30_Combat/enemy_approach_combat.h"
#include "Game/ActorBehavior/Enemy/P30_Combat/enemy_attack_combat.h"
#include "enemy_animation_controller.h"

class EnemyAiAgentSettingsAsset;

/// @brief 敵個体の状態判断、戦闘、移動、アニメーションを更新する司令塔。
class EnemyBehavior : public BehaviorComponent {
    EnemyContext m_context;

    EnemyLocomotionController m_locomotionController;
    EnemyMoveBehavior m_moveBehavior;
    EnemyConditionMachine m_conditionMachine;
    EnemyCombatTree m_combatTree;
    EnemyAnimationController m_animationController;

    // 最小構成のCombat。敵固有Combatへ置き換えられる。
    EnemyAttackCombat m_attackCombat;
    EnemyApproachCombat m_approachCombat;

    const EnemyAiAgentSettingsAsset* m_aiAgentSettings = nullptr;

public:
    EnemyBehavior() = default;
    ~EnemyBehavior() override = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    void SetupAiAgentSettings(const EnemyAiAgentSettingsAsset* settings) {
        m_aiAgentSettings = settings;
    }

    void StartStun(float duration) { m_conditionMachine.StartStun(duration); }
    EnemyCondition GetCondition() const { return m_conditionMachine.GetCurrentCondition(); }
    EnemyContext& GetContext() { return m_context; }
    const EnemyContext& GetContext() const { return m_context; }

private:
    void UpdateTargetState();
};
