#pragma once

#include <DirectXMath.h>

#include "Engine/Core/GamePlay/sequence_task.h"

class GameObject;
class HitStopBehavior;
class LightComponent;
class ParticleSystemComponent;
class RigidbodyComponent;
class SpriteAnimationComponent;

enum class PlayerAttackEffectType {
    AimHoldStart,
    AimStart,
    AimEnd,
    SingleAttack,
    SingleHit,
    ChargeStart,
    ChargeAttack,
    ChargeHit,
    AttackEnd,
};

class PlayerAttackEffects {
private:
    HitStopBehavior* m_hitStopBehavior = nullptr;
    RigidbodyComponent* m_rigidbody = nullptr;
    SpriteAnimationComponent* m_spriteAnimation = nullptr;
    ParticleSystemComponent* m_chargeEffect = nullptr;
    LightComponent* m_chargeLight = nullptr;

    bool m_lockMovement = false;

    class ChangeChargeLightTask : public SequenceTask {
    public:
        LightComponent* chargeLight = nullptr;

        DirectX::XMFLOAT4 defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4 targetColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4 endColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        float defaultIntensity = 0.0f;
        float startIntensity = 0.0f;
        float targetIntensity = 0.0f;
        float endIntensity = 0.0f;
        float duration = 0.0f;
        float holdDuration = 0.0f;

        void Start() override;
        void Update(float deltaTime) override;
    };

    ChangeChargeLightTask m_changeChargeLightTask;

    class CameraControlBehavior* m_cameraController = nullptr;

public:
    void Initialize(GameObject* owner);
    void Update(float unscaledDeltaTime);
    void Play(PlayerAttackEffectType type);

    void SetupChargeEffect(ParticleSystemComponent* chargeEffect);
    void SetupChargeLight(LightComponent* chargeLight);

    bool IsMovementLocked() const { return m_lockMovement; }

private:
    void ChangeChargeLight(const DirectX::XMFLOAT4& color, float intensity, float duration);
    void ChangeChargeLightTemporary(const DirectX::XMFLOAT4& color, float intensity, float duration, float holdDuration);
    void ResetChargeLight(float duration);
};
