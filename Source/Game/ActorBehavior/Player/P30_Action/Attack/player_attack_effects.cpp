#include "player_attack_effects.h"

#include <algorithm>

#include "Engine/Core/game_object.h"
#include "Engine/Framework/Component/light_component.h"
#include "Engine/Framework/Component/particle_system_component.h"
#include "Engine/Framework/Component/rigidbody_component.h"
#include "Engine/Framework/Component/sprite_animation_component.h"
#include "Game/ActorBehavior/Base/hit_stop_behavior.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/game_effect_controller.h"
#include "Utility/mi_math.h"

using namespace DirectX;

namespace {
    GameEffectController* GetGameEffect()
    {
        return GameControllerLocator::GetGameEffectController();
    }

    CustomPostEffectController* GetCustomPostEffect()
    {
        return GameControllerLocator::GetCustomPostEffectController();
    }
}

void PlayerAttackEffects::Initialize(GameObject* owner)
{
    if (!owner) return;

    m_hitStopBehavior = owner->GetComponent<HitStopBehavior>();
    m_rigidbody = owner->GetComponent<RigidbodyComponent>();
    m_spriteAnimation = owner->GetComponent<SpriteAnimationComponent>();
}

void PlayerAttackEffects::Update(float unscaledDeltaTime)
{
    m_changeChargeLightTask.Update(unscaledDeltaTime);
}

void PlayerAttackEffects::SetupChargeEffect(ParticleSystemComponent* chargeEffect)
{
    m_chargeEffect = chargeEffect;
}

void PlayerAttackEffects::SetupChargeLight(LightComponent* chargeLight)
{
    m_chargeLight = chargeLight;
    m_changeChargeLightTask.chargeLight = chargeLight;

    if (!m_chargeLight) return;

    m_changeChargeLightTask.defaultColor = m_chargeLight->GetDiffuse();
    m_changeChargeLightTask.defaultIntensity = m_chargeLight->GetIntensity();
}

void PlayerAttackEffects::Play(PlayerAttackEffectType type)
{
    GameEffectController* gameEffect = GetGameEffect();
    CustomPostEffectController* postEffect = GetCustomPostEffect();
    if (!gameEffect || !postEffect) return;

    switch (type) {
    case PlayerAttackEffectType::AimHoldStart:
        gameEffect->ChangeCameraLocalOffsetTemporary(XMFLOAT3(0.35f, 0.05f, 0.0f), 0.08f, 0.08f);
        break;

    case PlayerAttackEffectType::AimStart:
        gameEffect->ChangeFOV(65.0f, 0.2f);
        gameEffect->ChangeCameraOffset(XMFLOAT3(0.0f, -0.5f, 0.0f), 0.1f);
        gameEffect->ChangeCameraLocalOffset(XMFLOAT3(2.5f, 0.0f, 0.0f), 0.1f);
        gameEffect->ChangeCameraDistance(5.0f, 0.1f);
        postEffect->PlayEffect(CustomPostEffectType::MonoMask, 0.8f, 0.2f, MiMath::Infinity());
        break;

    case PlayerAttackEffectType::AimEnd:
        gameEffect->ResetFOV(0.1f);
        gameEffect->ResetCameraOffset(0.1f);
        gameEffect->ResetCameraLocalOffset(0.1f);
        gameEffect->ResetCameraDistance(0.1f);
        postEffect->PlayEffect(CustomPostEffectType::MonoMask, 0.0f, 0.1f, 0.0f);
        break;

    case PlayerAttackEffectType::SingleAttack:
        gameEffect->ChangeFOVTemporary(72.0f, 0.08f, 0.04f);
        gameEffect->ChangeCameraLocalOffsetTemporary(XMFLOAT3(0.45f, 0.0f, 0.10f), 0.06f, 0.04f);
        gameEffect->PlayCameraShake(0.10f, 0.15f);
        break;

    case PlayerAttackEffectType::SingleHit:
        gameEffect->PlayCameraShake(0.08f, 0.20f);
        break;

    case PlayerAttackEffectType::ChargeStart:
        gameEffect->ChangeFOV(60.0f, 0.2f);
        gameEffect->PlayCameraShake(0.08f, 0.10f);
        ChangeChargeLight(XMFLOAT4(0.35f, 0.75f, 1.0f, 1.0f), 8.0f, 1.0f);
        if (m_chargeEffect) {
            m_chargeEffect->Play();
            m_chargeEffect->Emission().enabled = true;
        }
        break;

    case PlayerAttackEffectType::ChargeAttack:
        if (m_hitStopBehavior) {
            m_hitStopBehavior->StartHitStop(
                0.2f,
                [this]() {
                    if (GameEffectController* effect = GetGameEffect()) {
                        effect->ChangeFOVTemporary(78.0f, 0.10f, 0.06f);
                        effect->ChangeCameraLocalOffsetTemporary(XMFLOAT3(0.15f, 0.0f, 0.25f), 0.08f, 0.08f);
                        effect->PlayCameraShake(0.16f, 0.30f);
                    }
                    if (CustomPostEffectController* effect = GetCustomPostEffect()) {
                        effect->PlayEffect(CustomPostEffectType::RadialBlur, 0.6f, 0.2f, MiMath::Infinity());
                    }
                    if (m_chargeLight) {
                        m_chargeLight->SetEnable(true);
                        m_chargeLight->SetIntensity(20.0f);
                    }
                    if (m_chargeEffect) {
                        m_chargeEffect->Stop();
                        m_chargeEffect->Emission().enabled = false;
                    }
                    m_lockMovement = true;
                    if (m_spriteAnimation) m_spriteAnimation->Stop();
                    if (m_rigidbody) m_rigidbody->SetIsKinematic(true);
                },
                nullptr,
                nullptr,
                [this]() {
                    if (CustomPostEffectController* effect = GetCustomPostEffect()) {
                        effect->PlayEffect(CustomPostEffectType::RadialBlur, 0.0f, 0.1f, 0.0f);
                    }
                    ResetChargeLight(0.5f);
                    if (m_chargeEffect) {
                        m_chargeEffect->Play();
                        m_chargeEffect->Emission().enabled = false;
                    }
                    m_lockMovement = false;
                    if (m_rigidbody) m_rigidbody->SetIsKinematic(false);
                });
        }
        break;

    case PlayerAttackEffectType::ChargeHit:
        gameEffect->ChangeFOVTemporary(82.0f, 0.06f, 0.04f);
        gameEffect->PlayCameraShake(0.14f, 0.40f);
        break;

    case PlayerAttackEffectType::AttackEnd:
        gameEffect->ResetFOV(0.12f);
        gameEffect->ResetCameraLocalOffset(0.12f);
        if (m_chargeEffect) m_chargeEffect->Stop();
        break;
    }
}

void PlayerAttackEffects::ChangeChargeLight(const XMFLOAT4& color, float intensity, float duration)
{
    if (!m_chargeLight) return;

    m_changeChargeLightTask.Reset();
    m_changeChargeLightTask.chargeLight = m_chargeLight;
    m_changeChargeLightTask.startColor = m_chargeLight->GetDiffuse();
    m_changeChargeLightTask.targetColor = color;
    m_changeChargeLightTask.endColor = color;
    m_changeChargeLightTask.startIntensity = m_chargeLight->GetIntensity();
    m_changeChargeLightTask.targetIntensity = intensity;
    m_changeChargeLightTask.endIntensity = intensity;
    m_changeChargeLightTask.duration = duration;
    m_changeChargeLightTask.holdDuration = 0.0f;
    m_changeChargeLightTask.Start();
}

void PlayerAttackEffects::ChangeChargeLightTemporary(
    const XMFLOAT4& color, float intensity, float duration, float holdDuration)
{
    if (!m_chargeLight) return;

    m_changeChargeLightTask.Reset();
    m_changeChargeLightTask.chargeLight = m_chargeLight;
    m_changeChargeLightTask.startColor = m_chargeLight->GetDiffuse();
    m_changeChargeLightTask.targetColor = color;
    m_changeChargeLightTask.endColor = m_changeChargeLightTask.defaultColor;
    m_changeChargeLightTask.startIntensity = m_chargeLight->GetIntensity();
    m_changeChargeLightTask.targetIntensity = intensity;
    m_changeChargeLightTask.endIntensity = m_changeChargeLightTask.defaultIntensity;
    m_changeChargeLightTask.duration = duration;
    m_changeChargeLightTask.holdDuration = holdDuration;
    m_changeChargeLightTask.Start();
}

void PlayerAttackEffects::ResetChargeLight(float duration)
{
    ChangeChargeLight(
        m_changeChargeLightTask.defaultColor,
        m_changeChargeLightTask.defaultIntensity,
        duration);
}

void PlayerAttackEffects::ChangeChargeLightTask::Start()
{
    SequenceTask::Start();
    if (chargeLight) {
        chargeLight->SetEnable(true);
        chargeLight->SetDiffuse(startColor);
        chargeLight->SetIntensity(startIntensity);
    }
}

void PlayerAttackEffects::ChangeChargeLightTask::Update(float deltaTime)
{
    if (IsFinished()) return;

    SequenceTask::Update(deltaTime);
    if (!chargeLight) {
        Finish();
        return;
    }

    switch (m_taskStep) {
    case 0: {
        const float t = duration > 0.0f ? (std::min)(m_taskTimer / duration, 1.0f) : 1.0f;
        chargeLight->SetDiffuse(MiMath::Lerp(startColor, targetColor, t));
        chargeLight->SetIntensity(MiMath::Lerp(startIntensity, targetIntensity, t));
        if (t >= 1.0f) {
            if (holdDuration > 0.0f) {
                AdvanceStep();
            }
            else {
                if (targetIntensity <= 0.0f) chargeLight->SetEnable(false);
                Finish();
            }
        }
        break;
    }
    case 1:
        if (Wait(holdDuration)) AdvanceStep();
        break;

    case 2: {
        const float t = duration > 0.0f ? (std::min)(m_taskTimer / duration, 1.0f) : 1.0f;
        chargeLight->SetDiffuse(MiMath::Lerp(targetColor, endColor, t));
        chargeLight->SetIntensity(MiMath::Lerp(targetIntensity, endIntensity, t));
        if (t >= 1.0f) {
            chargeLight->SetDiffuse(endColor);
            chargeLight->SetIntensity(endIntensity);
            if (endIntensity <= 0.0f) chargeLight->SetEnable(false);
            Finish();
        }
        break;
    }
    default:
        Finish();
        break;
    }
}
