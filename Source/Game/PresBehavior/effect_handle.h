// effect_handle.h
#pragma once

#include <limits>

class EffectHandle {
private:
    static constexpr unsigned int InvalidGameObjectID =
        (std::numeric_limits<unsigned int>::max)();

    class IScene* m_scene = nullptr;
    unsigned int m_gameObjectID = InvalidGameObjectID;

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
    void Reset();

    class GameObject* GetGameObject() const;
    class TransformComponent* GetTransform() const;

    class ParticleSystemComponent* GetParticleSystem() const;
    class MeshEffectComponent* GetMeshEffect() const;
};
