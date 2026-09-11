//---------------------------------------------------
// File  ：_/Enemy/enemy_behavior.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・敵全体を統括するBehavior。状態判断、戦闘、移動、アニメーションを更新する。
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_context.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move.h"
#include "Game/ActorBehavior/Enemy/E20_Condition/enemy_condition_machine.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_tree.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Approach/enemy_approach_combat.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/Melee/enemy_melee_attack_combat.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Wait/enemy_wait_combat.h"

#include "enemy_animation_controller.h"
#include "enemy_effects.h"
#include "enemy_motions.h"

class EnemyAiAgentSettingsAsset;
class EnemyMoveSettingsAsset;

/// @brief 敵個体の状態判断、戦闘、移動、アニメーションを更新する司令塔。
class EnemyBehavior : public BehaviorComponent {
private:
    bool m_registeredEntityToMetaAI = false;

    EnemyContext m_context;

    EnemyLocomotionController m_locomotionController;
    EnemyMove m_moveBehavior;
    EnemyConditionMachine m_conditionMachine;
    EnemyCombatTree m_combatTree;
    EnemyAnimationController m_animationController;
    EnemyEffects m_effects;
    EnemyMotions m_motions;

    EnemyMeleeAttackCombat m_attackCombat;
    EnemyWaitCombat m_waitCombat;
    EnemyApproachCombat m_approachCombat;

    const EnemyAiAgentSettingsAsset* m_aiAgentSettings = nullptr;
    const EnemyMoveSettingsAsset* m_moveSettings = nullptr;
    const EnemyApproachSettingsAsset* m_approachSettings = nullptr;
    const EnemyAttackSettingsAsset* m_attackSettings = nullptr;

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
    void SetupMoveSettings(const EnemyMoveSettingsAsset* settings) {
        m_moveSettings = settings;
    }
    void SetupApproachSettings(const EnemyApproachSettingsAsset* settings) {
        m_approachSettings = settings;
    }
    void SetupAttackSettings(const EnemyAttackSettingsAsset* settings) {
        m_attackSettings = settings;
    }

    // === Enemyへのアクセス ===
    void StartStun(float duration) { m_conditionMachine.StartStun(duration); }
    EnemyCondition GetCondition() const { return m_conditionMachine.GetCurrentCondition(); }
    EnemyContext& GetContext() { return m_context; }
    const EnemyContext& GetContext() const { return m_context; }

    void OnHitReceived(
        const HitReceiver::HitData& hitData,
        const HitReceiver::HitResult& hitResult);

private:
    void UpdateTargetState();

};
