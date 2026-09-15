#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
//===================================================
// File  ：_/Base/HitReceiver/hit_receiver_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "hit_receiver_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Device/mi_fps.h"

#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include "Game/ActorBehavior/Base/health_behavior.h"

using namespace HitReceiver;

void HitReceiverBehavior::Start()
{
    GameObject* owner = GetOwner();
    if (!owner) return;

    m_healthBehavior = owner->GetComponent<HealthBehavior>();
    m_damageReceiver.Initialize(m_healthBehavior);
    m_knockbackReceiver.Initialize(
        owner->GetComponent<TransformComponent>(),
        owner->GetComponent<RigidbodyComponent>());
}

void HitReceiverBehavior::Update()
{
    const float deltaTime = FPS_GetDeltaTime();

    m_knockbackReceiver.Update(deltaTime);
}

void HitReceiverBehavior::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "HitReceiverBehavior"))
    {
        ImGui::Text("HealthBehavior: %s", m_healthBehavior ? "true" : "false");
        ImGui::Text("Knockback Active: %s", IsKnockbackActive() ? "true" : "false");
    }

    BehaviorDetailView::EndSection();
}

/// @brief 攻撃を受け取る
HitResult HitReceiverBehavior::ReceiveHit(const HitData& hitData)
{
    // ダメージ処理
    HitResult result = m_damageReceiver.ReceiveDamage(hitData);
    if (!result.WasAccepted()) {
        return result;
    }

    if (auto* audio = Game::Audio()) {
        if (GetOwner()->GetTag() == "Player") audio->PlaySe(GameSe::PlayerHit);
        else if (GetOwner()->GetTag() == "Enemy") audio->PlaySe(GameSe::EnemyHit);
    }

    // ヒットリアクション、ノックバック開始処理
    if (!result.killed) {
        result.startedKnockback = m_knockbackReceiver.StartKnockback(hitData.knockback);
    }
    else {
        m_knockbackReceiver.CancelKnockback();
    }

    // ヒットリアクションの開始
    if (m_onHitCallback) {
        m_onHitCallback(hitData, result);
    }

    return result;
}

#pragma region Knockback
bool HitReceiverBehavior::IsKnockbackActive() const
{
    return m_knockbackReceiver.IsActive();
}

void HitReceiverBehavior::CancelKnockback()
{
    m_knockbackReceiver.CancelKnockback();
}
#pragma endregion
