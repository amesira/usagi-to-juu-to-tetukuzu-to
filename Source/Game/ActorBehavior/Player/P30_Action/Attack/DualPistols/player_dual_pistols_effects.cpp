//===================================================
// File  ：_/DualPistols/player_dual_pistols_effects.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_effects.h"
#include "player_dual_pistols_context.h"

#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/light_component.h"
#include "Engine/Component/mesh_effect_component.h"
#include "Engine/Component/transform_component.h"

#include "Game/Factory/render_effect_factory.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"

#include <filesystem>

namespace {
    const std::filesystem::path MUZZLE_FLASH_EFFECT_ASSET =
        "asset/MeshEffect/player_muzzle_flash_effect.mesh_effect.json";
    DirectX::XMFLOAT4 EulerToQuaternion(const DirectX::XMFLOAT3& rotation)
    {
        DirectX::XMFLOAT4 quaternion;
        DirectX::XMStoreFloat4(
            &quaternion,
            DirectX::XMQuaternionNormalize(
                DirectX::XMQuaternionRotationRollPitchYaw(
                    DirectX::XMConvertToRadians(rotation.x),
                    DirectX::XMConvertToRadians(rotation.y),
                    DirectX::XMConvertToRadians(rotation.z))));
        return quaternion;
    }
}

void PlayerDualPistolsEffects::Initialize(PlayerDualPistolsContext& context)
{
    if (m_initialized) return;
    m_initialized = true;

    m_slashBurstEffect1 = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        context.settings().slashBurstEffectAssetPath,
        EffectAttachmentDesc{
            .target = context.playerTransform,
        });
    m_slashBurstEffect2 = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        context.settings().slashBurstEffectAssetPath,
        EffectAttachmentDesc{
            .target = context.playerTransform,
        });
    m_slashBurstEffect3[0] = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        context.settings().slashBurstEffectAssetPath,
        EffectAttachmentDesc{
            .target = context.playerTransform,
        });
    m_slashBurstEffect3[1] = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        context.settings().slashBurstEffectAssetPath,
        EffectAttachmentDesc{
            .target = context.playerTransform,
        });

    ApplySlashBurstEffectTransforms(context);
}

void PlayerDualPistolsEffects::Update(PlayerDualPistolsContext& context)
{
    if (!m_initialized) return;

    // エディター上で変更されたSettingsAssetの値も再生中に反映する。
    ApplySlashBurstEffectTransforms(context);
}

void PlayerDualPistolsEffects::ApplySlashBurstEffectTransforms(PlayerDualPistolsContext& context)
{
    const auto& settings = context.settings();
    const DirectX::XMFLOAT3 commonScaling{
        settings.slashBurstEffectScale,
        settings.slashBurstEffectScale,
        settings.slashBurstEffectScale,
    };

    m_slashBurstEffect1.SetLocalTransform({
        .position = settings.slashBurst1Effect.position,
        .rotation = EulerToQuaternion(settings.slashBurst1Effect.rotation),
        .scaling = commonScaling,
    });
    m_slashBurstEffect2.SetLocalTransform({
        .position = settings.slashBurst2Effect.position,
        .rotation = EulerToQuaternion(settings.slashBurst2Effect.rotation),
        .scaling = commonScaling,
    });
    m_slashBurstEffect3[0].SetLocalTransform({
        .position = settings.slashBurst3LeftEffect.position,
        .rotation = EulerToQuaternion(settings.slashBurst3LeftEffect.rotation),
        .scaling = commonScaling,
    });
    m_slashBurstEffect3[1].SetLocalTransform({
        .position = settings.slashBurst3RightEffect.position,
        .rotation = EulerToQuaternion(settings.slashBurst3RightEffect.rotation),
        .scaling = commonScaling,
    });
}

void PlayerDualPistolsEffects::Finalize()
{
    
}

void PlayerDualPistolsEffects::PlayEffects(PlayerDualPistolsContext& context, EffectsType effectType)
{
    switch (effectType) {
        case EffectsType::FireLeft: {
            PlayMuzzleFlashEffect(context, 0);
            Game::GameFeedback()->PlayCameraShake(0.1f, 0.1f);
            break;
        }
        case EffectsType::FireRight: {
            PlayMuzzleFlashEffect(context, 1);
            Game::GameFeedback()->PlayCameraShake(0.1f, 0.1f);
            break;
        }
        case EffectsType::SlashBurst1: {
            m_slashBurstEffect1.Play();
            break;
        }
        case EffectsType::SlashBurst2: {
            m_slashBurstEffect2.Play();
            break;
        }
        case EffectsType::SlashBurst3: {
            m_slashBurstEffect3[0].Play();
            m_slashBurstEffect3[1].Play();
            break;
        }
        default: break;
    }
}

/// @brief マズルフラッシュエフェクトを再生する
void PlayerDualPistolsEffects::PlayMuzzleFlashEffect(PlayerDualPistolsContext& context, int pistolIndex)
{
    XMFLOAT3 spawnPosition = {};
    if (pistolIndex == 0) {
        spawnPosition = context.runtimeState.leftMuzzle.position;
        spawnPosition = MiMath::Add(
            spawnPosition, 
            MiMath::Multiply(
                MiMath::RotateVector(context.runtimeState.leftMuzzle.rotation, {0.0f, 1.0f, 0.0f}),
                2.0f));
    }
    else {
        spawnPosition = context.runtimeState.rightMuzzle.position;
        spawnPosition = MiMath::Add(
            spawnPosition, 
            MiMath::Multiply(
                MiMath::RotateVector(context.runtimeState.rightMuzzle.rotation, {0.0f, 1.0f, 0.0f}),
                2.0f));
    }

    for (int i = 0; i < m_muzzleFlashEffects.size(); i++) {
        if (!m_muzzleFlashEffects[i].IsPlaying()) {
            m_muzzleFlashEffects[i].GetTransform()->SetPosition(spawnPosition);
            m_muzzleFlashEffects[i].Play();
            return;
        }
    }

    // 最大数に達している場合は、最初のエフェクトを再利用する
    if (m_muzzleFlashEffects.size() >= 32) {
        m_muzzleFlashEffects[0].GetTransform()->SetPosition(spawnPosition);
        m_muzzleFlashEffects[0].Play();
        return;
    }

    // 新しいエフェクトを作成して再生する
    m_muzzleFlashEffects.push_back(RenderEffectFactory::CreateMeshEffect(
        context.scene,
        MUZZLE_FLASH_EFFECT_ASSET,
        { 
            .position = spawnPosition,
            .scaling = { 0.8f, 0.8f, 0.8f },
        }));
    m_muzzleFlashEffects.back().Play();
}
