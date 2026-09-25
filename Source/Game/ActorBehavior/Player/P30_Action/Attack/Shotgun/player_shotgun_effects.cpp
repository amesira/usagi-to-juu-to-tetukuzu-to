//===================================================
// File  ：_/Shotgun/player_shotgun_effects.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンのエフェクトを制御するクラス
//===================================================
#include "player_shotgun_effects.h"
#include "player_shotgun_context.h"

#include "Engine/Device/mi_fps.h"

#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/light_component.h"
#include "Engine/Component/mesh_effect_component.h"
#include "Engine/Component/transform_component.h"

#include "Game/Factory/render_effect_factory.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"

#include <filesystem>

namespace {
    const std::filesystem::path CHARGE_EFFECT_ASSET =
        "asset/Particle/player_shotgun_charging.particle.json";
    const std::filesystem::path CHARGE_COMPLETE_EFFECT_ASSET =
        "asset/MeshEffect/fresnel_sphere.mesh_effect.json";
    const std::filesystem::path MUZZLE_FLASH_EFFECT_ASSET =
        "asset/Particle/player_shotgun_fire.particle.json";

    float infinity = std::numeric_limits<float>::infinity();
}

void PlayerShotgunEffects::Initialize(PlayerShotgunContext& context)
{
    if (m_initialized) return;
    if (!context.scene || !context.playerTransform) return;

    // EffectHandleを作成して、ショットガンのエフェクトを初期化する
    m_chargeEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        context.scene,
        CHARGE_EFFECT_ASSET,
        EffectAttachmentDesc{
            .target = context.playerTransform,
            .localTransform = {
                .position = context.settings().chargeEffectOffset,
            },
        });

    m_chargeCompleteEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        CHARGE_COMPLETE_EFFECT_ASSET,
        EffectAttachmentDesc{
            .target = context.playerTransform,
            .localTransform = {
                .position = context.settings().chargeCompleteEffectOffset,
            },
        });
    const float completeEffectScale = context.settings().chargeCompleteEffectScale;
    m_chargeCompleteEffect.SetLocalScaling(
        { completeEffectScale, completeEffectScale, completeEffectScale });

    m_muzzleFlashEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        MUZZLE_FLASH_EFFECT_ASSET,
        EffectAttachmentDesc{
            .target = context.playerTransform,
            .localTransform = {
                .position = context.settings().muzzleFlashEffectOffset,
            },
        });

    // チャージ完了発射後のエフェクト終了時に呼び出すコールバックを設定
    m_chargeCompleteEffectTask.m_callback = [this]() {
        m_playChargeCompleteFireFinishEffect = true;
        };

    m_initialized = true;
}

void PlayerShotgunEffects::Update(PlayerShotgunContext& context)
{
    if (!m_initialized) return;

    float deltaTime = FPS_GetUnscaledDeltaTime();

    // === デバッグ用：Revision Counterが変化した場合、エフェクトの位置を更新する ===
    if (m_settingsRevisionCounter != context.settingsAsset->GetRevision()) {
        m_settingsRevisionCounter = context.settingsAsset->GetRevision();
        m_chargeEffect.SetLocalPosition(context.settings().chargeEffectOffset);

        const float completeEffectScale = context.settings().chargeCompleteEffectScale;
        m_chargeCompleteEffect.SetLocalTransform({
            .position = context.settings().chargeCompleteEffectOffset,
            .scaling = { completeEffectScale, completeEffectScale, completeEffectScale },
        });

        m_muzzleFlashEffect.SetLocalPosition(context.settings().muzzleFlashEffectOffset);
    }

    if (!m_chargeCompleteEffectTask.IsFinished()) {
        m_chargeCompleteEffectTask.Update(deltaTime);
    }

    if (m_playChargeCompleteFireFinishEffect) {
        m_playChargeCompleteFireFinishEffect = false;
        context.effects.PlayEffects(context, EffectsType::ChargeCompleteFireFinish);
    }
}

void PlayerShotgunEffects::Finalize()
{
    m_chargeEffect.Destroy();
    m_chargeCompleteEffect.Destroy();
    m_muzzleFlashEffect.Destroy();
    m_initialized = false;
}

void PlayerShotgunEffects::PlayEffects(PlayerShotgunContext& context, EffectsType effectType)
{
    switch (effectType) {
    // === エイムモード関連のエフェクト ===
    case EffectsType::AimEnter: {
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::RadialBlur,
            context.settings().aimEnterRadialBlurStrength,
            context.settings().aimTransitionTime,
            context.settings().aimRadialBlurHoldTime
        );
        Game::Audio()->PlaySe(GameSe::ShotgunAim);
        break;
    }
    case EffectsType::AimExit: {
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::RadialBlur,
            context.settings().aimExitRadialBlurStrength,
            context.settings().aimTransitionTime,
            context.settings().aimRadialBlurHoldTime
        );
        Game::Audio()->PlaySe(GameSe::ShotgunAim);
        break;
    }
    // === チャージ関連のエフェクト ===
    case EffectsType::StartCharge: {
        m_chargeEffect.Play();
        Game::GameFeedback()->ChangeFOV(context.settings().chargeFOV, context.settings().chargeStartTransitionTime);
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::MonoMask,
            context.settings().chargeMonoMaskStrength,
            context.settings().chargeStartTransitionTime,
            0.0f
        );
        m_chargeLoopSe = Game::Audio()->StartLoopSe(GameSe::Charge);
        break;
    }
    case EffectsType::ResetCharge: {
        m_chargeEffect.Stop();
        Game::GameFeedback()->ResetFOV(context.settings().chargeResetTransitionTime);
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::MonoMask, 0.0f, context.settings().chargeResetTransitionTime, 0.0f
        );
        Game::Audio()->StopLoopSe(m_chargeLoopSe);
        break;
    }
    case EffectsType::ChargeComplete: {
        m_chargeCompleteEffect.Play();
        Game::Audio()->PlaySe(GameSe::ChargeComplete);
        break;
    }
    // === 発射関連のエフェクト ===
    case EffectsType::Fire: {
        m_chargeEffect.Stop();
        m_muzzleFlashEffect.Play();
        Game::GameFeedback()->PlayCameraShake(0.2f, 1.0f);
        Game::Audio()->PlaySe(GameSe::Shotgun);

        PlayEffects(context, EffectsType::ResetCharge);
        break;
    }
    case EffectsType::ChargeCompleteFire: {
        m_chargeEffect.Stop();
        m_muzzleFlashEffect.Play();

        Game::GameFeedback()->ChangeFOVTemporary(
            context.settings().chargeFOV - 5.0f, 
            0.05f, infinity
        );
        Game::GameFeedback()->PlayCameraShake(0.2f, 1.5f);
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::RadialBlur,
            0.6f,
            0.05f, infinity
        );

        Game::GameFeedback()->ChangeTimeScaleTemporary(
            0.2f, 0.05f, infinity
        );

        Game::Audio()->PlaySe(GameSe::Shotgun);

        // ライトなどをフラッシュさせたい

        m_chargeCompleteEffectTask.m_waitDuration = context.settings().chargeCompleteFireEffectsHoldTime;
        m_chargeCompleteEffectTask.Start();
        break;
    }
    case EffectsType::ChargeCompleteFireFinish: {
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::RadialBlur,
            0.0f,
            0.1f, 0.0f
        );
        Game::GameFeedback()->ResetTimeScale(0.1f);

        PlayEffects(context, EffectsType::ResetCharge);
        break;
    }
    default: break;
    }
}
