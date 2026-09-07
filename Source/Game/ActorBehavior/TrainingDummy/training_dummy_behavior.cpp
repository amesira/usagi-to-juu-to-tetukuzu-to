//===================================================
// File  ：_/TrainingDummy/training_dummy_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "training_dummy_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"

#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"

using namespace HitReceiver;

void TrainingDummyBehavior::Start()
{
    m_transform = GetOwner()->GetComponent<TransformComponent>();
    m_rigidbody = GetOwner()->GetComponent<RigidbodyComponent>();
    m_healthBehavior = GetOwner()->GetComponent<HealthBehavior>();
    m_hitReceiverBehavior = GetOwner()->GetComponent<HitReceiverBehavior>();

    // ノックバック移動方法を指定
    if (m_hitReceiverBehavior) {
        m_hitReceiverBehavior->KnockbackReceiver()->SetDefaultMovementSource({
            KnockbackMovementMode::SetRigidbodyVelocity,
            true,
            -9.81f
            });
        m_hitReceiverBehavior->KnockbackReceiver()->SetLockKnockback(true);
        m_hitReceiverBehavior->SetOnHitCallback([this](const HitData& hitData, const HitResult& hitResult) {
            OnHitReceived(hitData, hitResult);
            });
    }

    m_rigidbody->SetIsKinematic(true);

    m_effects.Initialize(GetOwner());
    m_motions.Initialize(GetOwner());
}

void TrainingDummyBehavior::Update()
{
    float deltaTime = FPS_GetDeltaTime();

    m_effects.Update(deltaTime);
    m_motions.Update(deltaTime);

    // 練習用のかかしの振る舞いをここに実装する

    // ダメージを受けてから一定時間経過で回復

}

void TrainingDummyBehavior::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "TrainingDummyBehavior"))
    {
        ImGui::Text("This is a training dummy behavior.");
    }

    BehaviorDetailView::EndSection();
}

void TrainingDummyBehavior::OnHitReceived(const HitReceiver::HitData& hitData, const HitReceiver::HitResult& hitResult)
{
    if (!hitResult.WasAccepted()) return;

    if (!hitResult.killed) {
        m_effects.PlayHitEffects(hitData.hitPoint, hitData.hitDirection, hitResult.appliedDamage);
        m_effects.PlayConfusionEffects(0.5f);

        // powerRateはダメージ量に応じて0.0f~1.0fの範囲で設定する
        if (hitData.attackType == AttackType::Slash) {
            m_motions.PlaySlashHitMotion(hitData.hitDirection, 0.5f,
                hitData.hitStop.affectReceiver ? hitData.hitStop.duration : 0.0f);
        }
        else {
            m_motions.PlayKnockbackMotion(hitData.hitDirection, 0.5f, 0.2f);
        }

        m_effects.PlayFlashBlinkerEffect();
    }
    else {
        m_motions.CancelHitStop();
    }
}
