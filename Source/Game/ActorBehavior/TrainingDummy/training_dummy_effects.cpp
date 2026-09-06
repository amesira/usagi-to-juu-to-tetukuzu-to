//===================================================
// File  ：_/TrainingDummy/training_dummy_effects.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "training_dummy_effects.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/particle_system_component.h"

#include "Game/ActorBehavior/Base/ReactionEffects/blinker_behavior.h"

#include "Game/Factory/render_effect_factory.h"

#include "Utility/mi_math.h"

void TrainingDummyEffects::Initialize(GameObject* owner)
{
    m_transform = owner->GetComponent<TransformComponent>();
    m_blinkerBehavior = owner->GetComponent<BlinkerBehavior>();

    m_hitEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        owner->GetScene(),
        "asset/MeshEffect/hit_effect.mesh_effect.json",
        EffectAttachmentDesc{
            .target = m_transform,
        });
    m_confusionEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        owner->GetScene(),
        "asset/Particle/confusion_effect.particle.json",
        EffectAttachmentDesc{
            .target = m_transform,
            .localTransform = EffectTransform{
                .position = { 0.0f, 7.0f, 0.0f},
                .rotation = { 0.0f, 0.0f, 0.0f, 1.0f },
                .scaling = { 1.0f, 1.0f, 1.0f }
            },
        });
    m_confusionEffectRate = m_confusionEffect.GetParticleSystem()->Emission().rateOverTime;
}

void TrainingDummyEffects::Update(float deltaTime)
{
    // 混乱エフェクトのタイマー更新
    if (m_isActiveConfusionEffect) {
        m_confusionEffectTimer -= deltaTime;
        if (m_confusionEffectTimer <= 0.0f) {
            m_isActiveConfusionEffect = false;
            m_confusionEffect.GetParticleSystem()->Emission().rateOverTime = 0.0f;
        }
    }

    if (m_isActiveFlashBlinkerEffect && m_blinkerBehavior) {
        if (!m_blinkerBehavior->IsFlashing()) {
            m_isActiveFlashBlinkerEffect = false;
        }
    }
}

void TrainingDummyEffects::PlayHitEffects(const DirectX::XMFLOAT3& hitPosition, const DirectX::XMFLOAT3& hitDirection)
{
    DirectX::XMFLOAT3 localHitPosition = MiMath::Subtract(hitPosition, m_transform->GetPosition());
    m_hitEffect.SetLocalTransform(EffectTransform{
        .position = localHitPosition,
        .rotation = { 0.0f, 0.0f, 0.0f, 1.0f },
        .scaling = { 1.0f, 1.0f, 1.0f }
        });
    m_hitEffect.Play();
}

void TrainingDummyEffects::PlayConfusionEffects(float duration)
{
    m_confusionEffectTimer = duration;
    if (!m_isActiveConfusionEffect) {
        m_isActiveConfusionEffect = true;
        m_confusionEffect.GetParticleSystem()->Emission().rateOverTime = m_confusionEffectRate;
        m_confusionEffect.Play();
    }
}

void TrainingDummyEffects::PlayFlashBlinkerEffect()
{
    if (!m_isActiveFlashBlinkerEffect && m_blinkerBehavior) {
        m_isActiveFlashBlinkerEffect = true;
        m_blinkerBehavior->FlashTemporary({ 1.0f, 1.0f, 1.0f }, 0.8f, 0.05f, 0.01f);
    }
}
