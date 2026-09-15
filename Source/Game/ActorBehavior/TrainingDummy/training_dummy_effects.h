//---------------------------------------------------
// File  ：_/TrainingDummy/training_dummy_effects.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・TrainingDummyのエフェクトをまとめるクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/PresBehavior/effect_handle.h"
#include "Game/PresBehavior/attached_effect_handle.h"

class TrainingDummyEffects {
private:
    AttachedEffectHandle m_hitEffect;
    AttachedEffectHandle m_confusionEffect;

    class TransformComponent* m_transform = nullptr;
    class BlinkerBehavior* m_blinkerBehavior = nullptr;

    class IScene* m_scene = nullptr;

    bool m_isActiveConfusionEffect = false;
    float m_confusionEffectTimer = 0.0f;
    float m_confusionEffectRate = 10.0f; // 1秒あたりのパーティクル発生数

    bool m_isActiveFlashBlinkerEffect = false;

public:
    void Initialize(class GameObject* owner);
    void Update(float deltaTime);

    void PlayHitEffects(const DirectX::XMFLOAT3& hitPosition, const DirectX::XMFLOAT3& hitDirection, float damage = -1.0f);
    void PlayConfusionEffects(float duration);
    void PlayFlashBlinkerEffect();
};