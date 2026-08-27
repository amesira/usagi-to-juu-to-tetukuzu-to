//===================================================
// render_effect_factory.cpp
//
// Author：Miu Kitamura
// Date  ：2026/04/14
//===================================================
#include "render_effect_factory.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_base.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/decal_component.h"
#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/mesh_effect_component.h"
#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/transform_constraint_behavior.h"
#include "Engine/engine_service_locator.h"
#include "Utility/mi_string.h"

namespace
{
    bool ApplyParticleAsset(
        ParticleSystemComponent* particleSystem,
        const std::filesystem::path& assetPath,
        const std::wstring* textureOverride = nullptr)
    {
        if (!particleSystem) return false;

        ParticleSystemAssetLoader* loader = EngineServiceLocator::ParticleLoader();
        if (!loader) return false;

        ParticleSystemAsset* asset = loader->GetParticle(assetPath);
        if (!asset) return false;

        particleSystem->SetAsset(asset);
        particleSystem->GetDesc() = asset->GetDesc();

        auto& renderer = particleSystem->Renderer();
        TextureResource* texture = nullptr;
        TextureRepository* textureRepository = EngineServiceLocator::GetTextureRepository();
        if (textureRepository)
        {
            if (textureOverride)
            {
                particleSystem->SetTextureOverridePath(*textureOverride);
                texture = textureRepository->GetTextureResource(*textureOverride);
            }
            else if (!renderer.texturePath.empty())
            {
                particleSystem->ClearTextureOverridePath();
                texture = textureRepository->GetTextureResource(
                    MiString::ToWString(renderer.texturePath));
            }
        }
        particleSystem->SetTextureResource(texture);
        return true;
    }

    GameObject* CreateParticleObject(
        IScene* scene,
        const char* objectName,
        const XMFLOAT3& position,
        const std::filesystem::path& assetPath,
        const std::wstring* textureOverride = nullptr)
    {
        if (!scene) return nullptr;

        GameObject* effect = scene->CreateGameObject();
        effect->SetName(objectName);
        effect->SetRenderLayer(RenderLayer::Particle);

        TransformComponent* transform = effect->AddComponent<TransformComponent>();
        ParticleSystemComponent* particleSystem = effect->AddComponent<ParticleSystemComponent>();
        transform->SetPosition(position);

        if (!ApplyParticleAsset(particleSystem, assetPath, textureOverride))
        {
            EngineServiceLocator::AddLogMessage(
                "Failed to load particle asset: " + assetPath.generic_string());
        }
        return effect;
    }

    TransformConstraintBehavior* AttachToTransform(
        GameObject* effect,
        TransformComponent* target,
        const XMFLOAT3& offset)
    {
        if (!effect || !target) return nullptr;

        TransformConstraintBehavior* constraint =
            effect->AddComponent<TransformConstraintBehavior>();
        constraint->SetTarget(target);
        constraint->SetOffset(offset);
        constraint->SetConsiderRotation(true);
        constraint->SetConsiderScaling(false);
        return constraint;
    }
}

GameObject* RenderEffectFactory::CreateDecalEffect(
    IScene* scene,
    const XMFLOAT3& position,
    const std::wstring& decalTexturePath)
{
    if (!scene) return nullptr;

    GameObject* decalEffect = scene->CreateGameObject();
    decalEffect->SetName("DecalEffect");

    TransformComponent* transform = decalEffect->AddComponent<TransformComponent>();
    DecalComponent* decal = decalEffect->AddComponent<DecalComponent>();

    transform->SetPosition(position);
    if (TextureRepository* textureRepository = EngineServiceLocator::GetTextureRepository())
    {
        decal->SetDecalTexture(textureRepository->GetTextureResource(decalTexturePath));
    }
    return decalEffect;
}

EffectHandle RenderEffectFactory::CreateAttachedParticleEffect(
    IScene* scene,
    TransformComponent* target,
    const std::filesystem::path& assetPath,
    const XMFLOAT3& offset)
{
    if (!scene || !target) return {};

    GameObject* effect = CreateParticleObject(
        scene,
        "AttachedParticleEffect",
        target->GetPosition(),
        assetPath);
    if (!effect) return {};

    if (ParticleSystemComponent* particle = effect->GetComponent<ParticleSystemComponent>()) {
        particle->Main().playOnAwake = false;
        particle->Stop();
    }
    AttachToTransform(effect, target, offset);
    return EffectHandle(effect);
}

EffectHandle RenderEffectFactory::CreateAttachedMeshEffect(
    IScene* scene,
    TransformComponent* target,
    const std::filesystem::path& assetPath,
    const XMFLOAT3& offset)
{
    if (!scene || !target) return {};

    MeshEffectAssetLoader* loader = EngineServiceLocator::MeshEffectLoader();
    if (!loader) return {};

    MeshEffectAsset* asset = loader->Get(assetPath);
    if (!asset) {
        EngineServiceLocator::AddLogMessage(
            "Failed to load mesh effect asset: " + assetPath.generic_string());
        return {};
    }

    GameObject* effect = scene->CreateGameObject();
    effect->SetName("AttachedMeshEffect");
    effect->SetRenderLayer(RenderLayer::Particle);

    TransformComponent* transform = effect->AddComponent<TransformComponent>();
    transform->SetPosition(target->GetPosition());

    MeshEffectComponent* meshEffect = effect->AddComponent<MeshEffectComponent>();
    meshEffect->SetAsset(asset);
    meshEffect->Main().playOnAwake = false;
    meshEffect->Stop();

    AttachToTransform(effect, target, offset);
    return EffectHandle(effect);
}
