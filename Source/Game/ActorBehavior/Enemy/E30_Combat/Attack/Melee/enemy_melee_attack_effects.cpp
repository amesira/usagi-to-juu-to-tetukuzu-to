#include "enemy_melee_attack_effects.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_settings_asset.h"
#include "Game/Factory/render_effect_factory.h"

#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_context.h"

void EnemyMeleeAttackEffects::Initialize(const EnemyAttackContext& context)
{
    if (!context.scene || !context.transform || context.settings().slashEffectAssetPath.empty()) return;

    m_slashEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene, context.settings().slashEffectAssetPath, 
        EffectAttachmentDesc{
            .target = context.transform,
            .considerTargetRotation = true,
        });
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
}

void EnemyMeleeAttackEffects::SynchronizeAsset(const EnemyAttackContext& context)
{
    const int currentRevision = context.settingsAsset->GetRevision();
    if (m_revisionAsset != currentRevision) {
        m_revisionAsset = currentRevision;

        // ローカルTransformを設定する
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
    }
}
