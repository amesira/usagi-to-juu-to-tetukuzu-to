//===================================================
// File  ：_/Shotgun/player_shotgun_effects.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンのエフェクトを制御するクラス
//===================================================
#include "player_shotgun_effects.h"
#include "player_shotgun_context.h"

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
    const std::filesystem::path CHARGE_EFFECT_ASSET =
        "asset/Particle/player_shotgun_charging.particle.json";
    const std::filesystem::path CHARGE_COMPLETE_EFFECT_ASSET =
        "asset/MeshEffect/fresnel_sphere.mesh_effect.json";
    const std::filesystem::path MUZZLE_FLASH_EFFECT_ASSET =
        "asset/Particle/player_shotgun_fire.particle.json";

}

void PlayerShotgunEffects::Initialize(PlayerShotgunContext& context)
{
    if (m_initialized) return;
    if (!context.scene || !context.references.muzzleTransform) return;

    // EffectHandleを作成して、ショットガンのエフェクトを初期化する
    m_chargeEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        context.scene,
        context.references.muzzleTransform,
        CHARGE_EFFECT_ASSET,
        context.settings().chargeEffectOffset);

    m_chargeCompleteEffect = RenderEffectFactory::CreateAttachedMeshEffect(
        context.scene,
        context.references.muzzleTransform,
        CHARGE_COMPLETE_EFFECT_ASSET,
        context.settings().chargeCompleteEffectOffset);
    const float completeEffectScale = context.settings().chargeCompleteEffectScale;
    m_chargeCompleteEffect.GetTransform()->SetScaling(
        { completeEffectScale, completeEffectScale, completeEffectScale });

    m_muzzleFlashEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        context.scene,
        context.references.muzzleTransform,
        MUZZLE_FLASH_EFFECT_ASSET,
        context.settings().muzzleFlashEffectOffset);

    m_initialized = true;
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
        break;
    }
    case EffectsType::AimExit: {
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::RadialBlur,
            context.settings().aimExitRadialBlurStrength,
            context.settings().aimTransitionTime,
            context.settings().aimRadialBlurHoldTime
        );
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
        break;
    }
    case EffectsType::ResetCharge: {
        m_chargeEffect.Stop();
        m_chargeCompleteEffect.Stop();
        Game::GameFeedback()->ResetFOV(context.settings().chargeResetTransitionTime);
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::MonoMask, 0.0f, context.settings().chargeResetTransitionTime, 0.0f
        );
        break;
    }
    case EffectsType::ChargeComplete: {
        m_chargeCompleteEffect.Play();
        break;
    }
    case EffectsType::Fire: {
        m_chargeEffect.Stop();
        m_chargeCompleteEffect.Stop();
        m_muzzleFlashEffect.Play();
        break;
    }
    default: break;
    }
}
