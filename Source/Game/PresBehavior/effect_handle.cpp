// effect_handle.cpp
#include "effect_handle.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_base.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/mesh_effect_component.h"

EffectHandle::EffectHandle(GameObject* gameObject)
{
    if (gameObject) {
        m_scene = gameObject->GetScene();
        m_gameObjectID = gameObject->GetID();
    }
}

/// @brief 有効なEffectHandleかどうかを判定する
bool EffectHandle::IsValid() const
{
    if (!m_scene) return false;
    GameObject* gameObject = m_scene->GetGameObjectByID(m_gameObjectID);
    return gameObject != nullptr;
}

#pragma region 再生関連
void EffectHandle::Play()
{
    if (!IsValid()) return;
    if (ParticleSystemComponent* particleSystem = GetParticleSystem()) {
        particleSystem->Play();
    }
    if (MeshEffectComponent* meshEffect = GetMeshEffect()) {
        meshEffect->Play();
    }
}

void EffectHandle::Stop()
{
    if (!IsValid()) return;
    if (ParticleSystemComponent* particleSystem = GetParticleSystem()) {
        particleSystem->Stop();
    }
    if (MeshEffectComponent* meshEffect = GetMeshEffect()) {
        meshEffect->Stop();
    }
}

void EffectHandle::Pause()
{
    if (!IsValid()) return;
    if (ParticleSystemComponent* particleSystem = GetParticleSystem()) {
        particleSystem->Pause();
    }
    if (MeshEffectComponent* meshEffect = GetMeshEffect()) {
        meshEffect->Pause();
    }
}
#pragma endregion

void EffectHandle::SetActive(bool active)
{
    if (!IsValid()) return;
    if (GameObject* gameObject = GetGameObject()) {
        gameObject->SetActive(active);
    }
}

void EffectHandle::Destroy()
{
    if (!IsValid()) return;
    if (GameObject* gameObject = GetGameObject()) {
        gameObject->Destroy();
    }
}

#pragma region GameObject/Component取得関連
GameObject* EffectHandle::GetGameObject() const
{
    if (!IsValid()) return nullptr;
    return m_scene->GetGameObjectByID(m_gameObjectID);
}

TransformComponent* EffectHandle::GetTransform() const
{
    GameObject* gameObject = GetGameObject();
    if (!gameObject) return nullptr;
    return gameObject->GetComponent<TransformComponent>();
}

ParticleSystemComponent* EffectHandle::GetParticleSystem() const
{
    GameObject* gameObject = GetGameObject();
    if (!gameObject) return nullptr;
    return gameObject->GetComponent<ParticleSystemComponent>();
}

MeshEffectComponent* EffectHandle::GetMeshEffect() const
{
    GameObject* gameObject = GetGameObject();
    if (!gameObject) return nullptr;
    return gameObject->GetComponent<MeshEffectComponent>();
}
#pragma endregion