// effect_handle.h
#pragma once

class EffectHandle {
private:
    class IScene* m_scene = nullptr;
    unsigned int m_gameObjectID = 0;

public:
    EffectHandle() = default;
    EffectHandle(class GameObject* gameObject);

    /// @brief 有効なEffectHandleかどうかを判定する
    bool IsValid() const;

    void Play();
    void Stop();
    void Pause();

    void SetActive(bool active);
    void Destroy();

    class GameObject* GetGameObject() const;
    class TransformComponent* GetTransform() const;

    class ParticleSystemComponent* GetParticleSystem() const;
    class MeshEffectComponent* GetMeshEffect() const;
};