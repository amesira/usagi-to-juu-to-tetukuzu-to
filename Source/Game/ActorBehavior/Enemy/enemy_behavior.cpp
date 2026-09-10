//===================================================
// File  ：_/Enemy/enemy_behavior.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/transform_component.h"
#include "External/ImGui/imgui.h"
#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"

void EnemyBehavior::Start()
{
    GameObject* owner = GetOwner();
    if (!owner) return;

    m_context.owner = this;
    m_context.scene = owner->GetScene();
    m_context.transform = owner->GetComponent<TransformComponent>();
    m_context.rigidbody = owner->GetComponent<RigidbodyComponent>();
    m_context.health = owner->GetComponent<HealthBehavior>();
    m_context.aiWorld = Game::EnemyAIWorld();
    m_context.aiAgentSettingsAsset = m_aiAgentSettings;

    m_context.locomotionController = &m_locomotionController;
    m_context.moveBehavior = &m_moveBehavior;
    m_context.conditionMachine = &m_conditionMachine;
    m_context.combatTree = &m_combatTree;
    m_context.animationController = &m_animationController;

    m_locomotionController.Initialize();
    m_moveBehavior.Initialize(m_context, m_moveSettings);
    m_approachCombat.Initialize(m_context, m_approachSettings);
    m_combatTree.Initialize(m_context);

    m_combatTree.RegisterBehavior(m_attackCombat);
    m_combatTree.RegisterBehavior(m_approachCombat);

    m_animationController.Initialize(m_context);

    UpdateTargetState();
    m_conditionMachine.Initialize(m_context);

    // 敵個体をMetaAIに登録する
    Game::EnemyAIWorld()->GetMetaAI().RegisterEnemy(owner);
}

void EnemyBehavior::Update()
{
    const float deltaTime = FPS_GetDeltaTime();

    // ターゲットの確認、状態の更新
    UpdateTargetState();
    m_conditionMachine.Update(m_context, deltaTime);

    // Combat状態の時のみCombatTreeを更新する
    // Approachの停止距離を共有し、射程との設定の食い違いを防ぐ。
    m_attackCombat.SetAttackDistance(m_approachCombat.GetStopDistance());
    if (m_conditionMachine.IsCombat()) {
        m_combatTree.Update(m_context, deltaTime);
    }
    else {
        m_combatTree.Cancel(m_context);
    }

    // 移動の更新
    const EnemyMoveIntent intent = m_locomotionController.BuildIntent(m_context);
    m_moveBehavior.UpdateMove(m_context, intent, deltaTime);

    // アニメーションの更新
    m_animationController.Update(m_context);
}

void EnemyBehavior::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Enemy Behavior")) {
        ImGui::Text("Condition: %d", static_cast<int>(m_conditionMachine.GetCurrentCondition()));
        ImGui::Text("Has Target: %s", m_context.runtimeState.hasCombatTarget ? "true" : "false");
        ImGui::Text("Velocity: %.2f, %.2f, %.2f",
            m_context.runtimeState.controlVelocity.x,
            m_context.runtimeState.controlVelocity.y,
            m_context.runtimeState.controlVelocity.z);
    }
    BehaviorDetailView::EndSection();
}

/// @brief 敵のターゲット状態を更新する
void EnemyBehavior::UpdateTargetState()
{
    m_context.aiWorld = Game::EnemyAIWorld();
    const bool hasTarget = m_context.aiWorld
        && m_context.aiWorld->GetEnable()
        && m_context.aiWorld->IsInitialized()
        && m_context.aiWorld->GetMetaAI().HasPlayer();

    m_context.runtimeState.hasCombatTarget = hasTarget;
    if (hasTarget) {
        m_context.runtimeState.combatTargetPosition =
            m_context.aiWorld->GetMetaAI().GetPlayerPosition();
    }
}
