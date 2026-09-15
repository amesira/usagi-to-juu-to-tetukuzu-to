#pragma once

#include "effect_handle.h"
#include "effect_transform.h"

/// @brief Transformへ追従するエフェクトを操作するハンドル
/// @details 追従に使用するコンポーネントの種類は公開インターフェースへ露出させない。
class AttachedEffectHandle {
private:
    EffectHandle m_effect;

public:
    AttachedEffectHandle() = default;
    explicit AttachedEffectHandle(const EffectHandle& effect) : m_effect(effect) {}

    bool IsValid() const { return m_effect.IsValid(); }

    void Play() { m_effect.Play(); }
    void Stop() { m_effect.Stop(); }
    void Pause() { m_effect.Pause(); }
    void SetActive(bool active) { m_effect.SetActive(active); }
    void Destroy() { m_effect.Destroy(); }
    void Reset() { m_effect.Reset(); }

    bool IsPlaying() const { return m_effect.IsPlaying(); }

    void SetLocalTransform(const EffectTransform& transform);
    void SetLocalPosition(const DirectX::XMFLOAT3& position);
    void SetLocalRotation(const DirectX::XMFLOAT4& rotation);
    void SetLocalScaling(const DirectX::XMFLOAT3& scaling);

    const EffectHandle& GetEffect() const { return m_effect; }
    EffectHandle& GetEffect() { return m_effect; }

    class GameObject* GetGameObject() const { return m_effect.GetGameObject(); }
    class TransformComponent* GetTransform() const { return m_effect.GetTransform(); }
    class ParticleSystemComponent* GetParticleSystem() const { return m_effect.GetParticleSystem(); }
    class MeshEffectComponent* GetMeshEffect() const { return m_effect.GetMeshEffect(); }
};
