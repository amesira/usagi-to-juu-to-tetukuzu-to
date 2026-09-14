#pragma once

#include <DirectXMath.h>

#include "Game/PresBehavior/attached_effect_handle.h"

class EnemyEffects {
private:
    AttachedEffectHandle m_hitEffect;
    class TransformComponent* m_transform = nullptr;
    class BlinkerBehavior* m_blinkerBehavior = nullptr;
    class IScene* m_scene = nullptr;
    bool m_isFlashing = false;

public:
    void Initialize(class GameObject* owner);
    void Update(float deltaTime);
    void Stop();
    void Finalize();

    void PlayHitEffects(
        const DirectX::XMFLOAT3& hitPosition,
        const DirectX::XMFLOAT3& hitDirection,
        float damage);
    void PlayFlashBlinkerEffect();
};
