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
#include "Engine/Component/collider_component.h"
#include "External/ImGui/imgui.h"

#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Game/ControllerBehavior/Wave/wave_controller_behavior.h"

using namespace HitReceiver;

void EnemyBehavior::Start()
{
    GameObject* owner = GetOwner();
    if (!owner) return;

    m_context.owner = this;
    m_context.scene = owner->GetScene();
    m_context.transform = owner->GetComponent<TransformComponent>();
    m_context.rigidbody = owner->GetComponent<RigidbodyComponent>();
    m_context.health = owner->GetComponent<HealthBehavior>();
    m_context.hitReceiver = owner->GetComponent<HitReceiverBehavior>();
    m_context.aiWorld = Game::EnemyAIWorld();
    m_context.aiAgentSettingsAsset = m_useInstanceAiAgentSettings ? &m_instanceAiAgentSettings : m_aiAgentSettings;

    m_context.locomotionController = &m_locomotionController;
    m_context.moveBehavior = &m_moveBehavior;
    m_context.conditionMachine = &m_conditionMachine;
    m_context.combatTree = &m_combatTree;
    m_context.animationController = &m_animationController;

    if (m_context.hitReceiver) {
        m_context.hitReceiver->KnockbackReceiver()->SetDefaultMovementSource({
            KnockbackMovementMode::SetRigidbodyVelocity,
            -9.81f * 2.0f,
        });
        m_context.hitReceiver->SetOnHitCallback(
            [this](const HitData& hitData, const HitResult& hitResult) {
                OnHitReceived(hitData, hitResult);
            });

        if (m_isElite) {
            m_context.hitReceiver->KnockbackReceiver()->SetLockKnockback(true);
        }
    }

    m_context.health->SetUiOffset({ 0.0f, m_context.transform->GetScaling().y * 3.0f, 0.0f });

    m_effects.Initialize(owner);
    const bool modelB = m_attackType == EnemyAttackType::Ranged;
    m_motions.Initialize(owner, modelB);
    m_animationController.SetModelB(modelB);

    m_locomotionController.Initialize();
    m_moveBehavior.Initialize(m_context, m_moveSettings);
    m_approachCombat.Initialize(m_context, m_approachSettings);
    m_attackCombat = m_attackType == EnemyAttackType::Ranged
        ? static_cast<EnemyAttackCombat*>(&m_rangedAttackCombat)
        : static_cast<EnemyAttackCombat*>(&m_meleeAttackCombat);
    m_attackCombat->SetSettingsAsset(m_attackSettings);
    m_combatTree.Initialize(m_context);
    m_attackCombat->Initialize(m_context);
    m_waitCombat.Initialize(m_context);

    m_combatTree.RegisterBehavior(*m_attackCombat);
    m_combatTree.RegisterBehavior(m_waitCombat);
    m_combatTree.RegisterBehavior(m_approachCombat);

    m_animationController.Initialize(m_context);

    UpdateTargetState();
    m_conditionMachine.Initialize(m_context);

    // 敵個体をMetaAIに登録する
    auto* aiWorld = Game::EnemyAIWorld();
    if (aiWorld && aiWorld->GetEnable() && aiWorld->IsInitialized()) {
        aiWorld->GetMetaAI().RegisterEnemy(owner);
        m_registeredEntityToMetaAI = true;
    }
    else {
        m_registeredEntityToMetaAI = false;
    }
}

void EnemyBehavior::Update()
{
    const float deltaTime = FPS_GetDeltaTime();

    m_effects.Update(deltaTime);
    m_motions.Update(deltaTime);

    if (!m_registeredEntityToMetaAI) {
        auto* aiWorld = Game::EnemyAIWorld();
        if (aiWorld && aiWorld->GetEnable() && aiWorld->IsInitialized()) {
            aiWorld->GetMetaAI().RegisterEnemy(GetOwner());
            m_registeredEntityToMetaAI = true;
        }
    }

    // ターゲットの確認、状態の更新
    UpdateTargetState();
    m_conditionMachine.Update(m_context, deltaTime);

    // Combat状態の時のみCombatTreeを更新する
    if (m_conditionMachine.IsCombat()) {
        m_combatTree.Update(m_context, deltaTime);
    }
    else {
        m_combatTree.Cancel(m_context);
        m_context.runtimeState.isInAttackRange = false;
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

void EnemyBehavior::OnHitReceived(const HitData& hitData, const HitResult& hitResult)
{
    if (!hitResult.WasAccepted()) return;

    if (hitResult.killed) {
        if (m_deathReason == EnemyDeathReason::Defeated) {
            if (auto* wave = Game::Wave()) wave->NotifyEnemyDefeated(GetOwner()->GetID());
        }
        m_motions.Stop();
        return;
    }

    m_effects.PlayHitEffects(hitData.hitPoint, hitData.hitDirection, hitResult.appliedDamage);
    m_effects.PlayFlashBlinkerEffect();

    if (hitData.attackType == AttackType::Slash) {
        m_motions.PlaySlashHitMotion(
            hitData.hitDirection,
            1.0f,
            hitData.hitStop.affectReceiver ? hitData.hitStop.duration : 0.0f);
    }
    else {
        m_motions.PlayKnockbackMotion(hitData.hitDirection, 1.0f, 0.2f);
    }

    // エリート敵はスタンしないようにする
    if (!m_isElite){
        const float stunDuration = hitData.knockback.enabled
            ? hitData.knockback.duration
            : 0.2f;
        StartStun(stunDuration);
    }
}

void EnemyBehavior::RequestWaveCleanup()
{
    auto* owner = GetOwner();
    auto* health = owner ? owner->GetComponent<HealthBehavior>() : nullptr;
    if (!health || health->IsDead()) return;
    // Mark before triggering death, so cleanup can never be credited as a hit.
    m_deathReason = EnemyDeathReason::WaveCleanup;
    m_motions.Stop();
    m_combatTree.Cancel(m_context);
    if (m_context.hitReceiver) m_context.hitReceiver->SetEnable(false);
    if (auto* collider = owner->GetComponent<CapsuleColliderComponent>()) collider->SetEnable(false);
    health->SetHealth(0);
    health->SetUiActive(false);
    m_conditionMachine.Update(m_context, 0);
}
