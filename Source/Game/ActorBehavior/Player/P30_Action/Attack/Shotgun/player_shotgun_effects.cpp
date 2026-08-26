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

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"

void PlayerShotgunEffects::PlayEffects(PlayerShotgunContext& context, EffectsType effectType)
{
    switch (effectType) {
    // === エイムモード関連のエフェクト ===
    case EffectsType::AimEnter: {
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::RadialBlur, 0.5f, context.settings().aimTransitionTime, 0.01f
        );
        break;
    }

    // === チャージ関連のエフェクト ===
    case EffectsType::StartCharge: {
        if (context.references.chargeParticle) {
            context.references.chargeParticle->Play();
        }
        if (context.references.chargeLight) {
            context.references.chargeLight->SetIntensity(context.settings().chargeLightIntensity);
        }
        Game::GameFeedback()->ChangeFOV(context.settings().chargeFOV, context.settings().chargeStartTransitionTime);
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::MonoMask, 0.6f, context.settings().chargeStartTransitionTime, 0.0f
        );
        break;
    }
    case EffectsType::ResetCharge: {
        if (context.references.chargeParticle) {
            context.references.chargeParticle->Stop();
        }
        if (context.references.chargeLight) {
            context.references.chargeLight->SetIntensity(0.0f);
        }
        Game::GameFeedback()->ResetFOV(context.settings().chargeResetTransitionTime);
        Game::CustomPostEffect()->PlayEffect(
            CustomPostEffectType::MonoMask, 0.0f, context.settings().chargeResetTransitionTime, 0.0f
        );
        break;
    }
    default: break;
    }
}
