#include "enemy_melee_attack_effects.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_settings_asset.h"
#include "Game/Factory/render_effect_factory.h"

#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_context.h"

#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/particle_system_component.h"

void EnemyMeleeAttackEffects::Initialize(const EnemyAttackContext& context)
{
    if (!context.scene || !context.transform || context.settings().slashEffectAssetPath.empty()) return;

    m_slashEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene, context.settings().slashEffectAssetPath, 
        EffectAttachmentDesc{
            .target = context.transform,
            .considerTargetRotation = true,
        });

    m_windupEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        context.scene, "asset/Particle/enemy_charge.particle.json",
        EffectAttachmentDesc{
            .target = context.transform,
            .considerTargetRotation = true,
        });

    // 新しく生成したエフェクトには、AssetのRevisionが同じでもTransformを適用する。
    m_revisionAsset = (std::numeric_limits<std::uint64_t>::max)();
    SynchronizeAsset(context);
}

void EnemyMeleeAttackEffects::Update(const EnemyAttackContext& context)
{
    if (!m_slashEffect.IsValid()) return;

    // アセットの更新があれば、ローカルTransformを再設定する
    SynchronizeAsset(context);
}

void EnemyMeleeAttackEffects::PlayEffects(EffectsType type)
{
    switch (type) {
    case EffectsType::Slash:{
        m_slashEffect.Play();
        Game::Audio()->PlaySe(GameSe::EnemySlash);
        break;
    }
    case EffectsType::BeginWindup: {
        m_windupEffect.GetParticleSystem()->Emission().enabled = true;
        m_windupEffect.Play();
        break;
    }
    case EffectsType::EndWindup: {
        m_windupEffect.GetParticleSystem()->Emission().enabled = false;
        break;
    }
    default: break;
    }
}

void EnemyMeleeAttackEffects::StopSlash() { 
    m_slashEffect.Stop();
}

void EnemyMeleeAttackEffects::Finalize()
{
    m_slashEffect.Stop();
    m_slashEffect.Destroy();
    m_revisionAsset = (std::numeric_limits<std::uint64_t>::max)();
}

void EnemyMeleeAttackEffects::SynchronizeAsset(const EnemyAttackContext& context)
{
    const std::uint64_t currentRevision = context.settingsAsset->GetRevision();
    if (m_revisionAsset != currentRevision) {
        m_revisionAsset = currentRevision;

        // SlashEffect
        DirectX::XMFLOAT4 rotation;
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationRollPitchYaw(
            DirectX::XMConvertToRadians(context.settings().slashEffectRotation.x),
            DirectX::XMConvertToRadians(context.settings().slashEffectRotation.y),
            DirectX::XMConvertToRadians(context.settings().slashEffectRotation.z)));
        const float scale = context.settings().slashEffectScale;
        m_slashEffect.SetLocalTransform({
            .position = context.settings().slashEffectPosition,
            .rotation = rotation,
            .scaling = { scale, scale, scale },
            });

        // WindupEffect
        DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionRotationRollPitchYaw(
            DirectX::XMConvertToRadians(context.settings().windupEffectRotation.x),
            DirectX::XMConvertToRadians(context.settings().windupEffectRotation.y),
            DirectX::XMConvertToRadians(context.settings().windupEffectRotation.z)));
        m_windupEffect.SetLocalTransform({
            .position = context.settings().windupEffectPosition,
            .rotation = rotation,
            .scaling = { 1.0f, 1.0f, 1.0f },
            });
    }
}
