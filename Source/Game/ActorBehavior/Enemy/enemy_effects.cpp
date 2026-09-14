#include "enemy_effects.h"

#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Base/ReactionEffects/blinker_behavior.h"
#include "Game/Factory/projectile_factory.h"
#include "Game/Factory/render_effect_factory.h"
#include "Utility/mi_math.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"

void EnemyEffects::Initialize(GameObject* owner)
{
    if (!owner) return;

    m_transform = owner->GetComponent<TransformComponent>();
    m_blinkerBehavior = owner->GetComponent<BlinkerBehavior>();
    m_scene = owner->GetScene();

    m_hitEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        m_scene,
        "asset/MeshEffect/hit_effect.mesh_effect.json",
        EffectAttachmentDesc{ .target = m_transform });
}

void EnemyEffects::Update(float)
{
    if (m_isFlashing && m_blinkerBehavior && !m_blinkerBehavior->IsFlashing()) {
        m_isFlashing = false;
    }
}

void EnemyEffects::PlayHitEffects(
    const DirectX::XMFLOAT3& hitPosition,
    const DirectX::XMFLOAT3& hitDirection,
    float damage)
{
    if (!m_transform) return;

    XMFLOAT3 effectPosition = hitPosition;
    effectPosition = MiMath::Add(effectPosition, MiMath::Multiply(hitDirection, -1.5f));

    m_hitEffect.SetLocalTransform(EffectTransform{
        .position = MiMath::Subtract(effectPosition, m_transform->GetPosition()),
        .rotation = { 0.0f, 0.0f, 0.0f, 1.0f },
        .scaling = { 1.0f, 1.0f, 1.0f }
    });
    m_hitEffect.Play();

    XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (damage > 30.0f) {
        color = { 1.0f, 1.0f, 0.0f, 1.0f };
    }

    if (m_scene && damage > 0.0f) {
        ProjectileFactory::CreateDamageNumber(m_scene, ProjectileFactory::DamageNumberCreateDesc{
            .position = effectPosition,
            .damage = damage,
            .color = color
        });
    }

    Game::Audio()->PlaySe(GameSe::EnemyHit);
}

void EnemyEffects::PlayFlashBlinkerEffect()
{
    if (m_isFlashing || !m_blinkerBehavior) return;

    m_isFlashing = true;
    m_blinkerBehavior->FlashTemporary({ 1.0f, 1.0f, 1.0f }, 0.8f, 0.05f, 0.01f);
}
