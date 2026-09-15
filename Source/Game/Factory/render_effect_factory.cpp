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
#include "Game/PresBehavior/one_shot_particle_behavior.h"
#include "Engine/engine_service_locator.h"
#include "Utility/mi_string.h"

namespace RenderEffectFactory
{
    bool ApplyParticleAsset(
        ParticleSystemComponent* particleSystem,
        const std::filesystem::path& assetPath,
        const std::wstring* textureOverride)
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
}

namespace
{
    GameObject* CreateParticleObject(
        IScene* scene,
        const char* objectName,
        const std::filesystem::path& assetPath,
        const EffectTransform& effectTransform,
        const std::wstring* textureOverride = nullptr)
    {
        if (!scene) return nullptr;

        GameObject* effect = scene->CreateGameObject();
        effect->SetName(objectName);
        effect->SetRenderLayer(RenderLayer::Particle);

        TransformComponent* transform = effect->AddComponent<TransformComponent>();
        ParticleSystemComponent* particleSystem = effect->AddComponent<ParticleSystemComponent>();
        transform->SetPosition(effectTransform.position);
        transform->SetRotation(effectTransform.rotation);
        transform->SetScaling(effectTransform.scaling);

        if (!RenderEffectFactory::ApplyParticleAsset(particleSystem, assetPath, textureOverride))
        {
            EngineServiceLocator::AddLogMessage(
                "Failed to load particle asset: " + assetPath.generic_string());
        }
        return effect;
    }

    TransformConstraintBehavior* AttachToTransform(
        GameObject* effect,
        const EffectAttachmentDesc& attachment)
    {
        if (!effect || !attachment.target) return nullptr;

        TransformConstraintBehavior* constraint =
            effect->AddComponent<TransformConstraintBehavior>();
        constraint->SetTarget(attachment.target);
        constraint->SetLocalPosition(attachment.localTransform.position);
        constraint->SetLocalRotation(attachment.localTransform.rotation);
        constraint->SetLocalScaling(attachment.localTransform.scaling);
        constraint->SetConsiderRotation(attachment.considerTargetRotation);
        constraint->SetConsiderScaling(attachment.considerTargetScaling);
        return constraint;
    }

    GameObject* CreateMeshEffectObject(
        IScene* scene,
        const char* objectName,
        const std::filesystem::path& assetPath,
        const EffectTransform& effectTransform)
    {
        if (!scene) return nullptr;

        MeshEffectAssetLoader* loader = EngineServiceLocator::MeshEffectLoader();
        if (!loader) return nullptr;

        MeshEffectAsset* asset = loader->Get(assetPath);
        if (!asset) {
            EngineServiceLocator::AddLogMessage(
                "Failed to load mesh effect asset: " + assetPath.generic_string());
            return nullptr;
        }

        GameObject* effect = scene->CreateGameObject();
        effect->SetName(objectName);
        effect->SetRenderLayer(RenderLayer::Particle);

        TransformComponent* transform = effect->AddComponent<TransformComponent>();
        transform->SetPosition(effectTransform.position);
        transform->SetRotation(effectTransform.rotation);
        transform->SetScaling(effectTransform.scaling);

        MeshEffectComponent* meshEffect = effect->AddComponent<MeshEffectComponent>();
        meshEffect->SetAsset(asset);
        return effect;
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

EffectHandle RenderEffectFactory::CreateParticleEffect(
    IScene* scene,
    const std::filesystem::path& assetPath,
    const EffectTransform& transform)
{
    GameObject* effect = CreateParticleObject(
        scene,
        "ParticleEffect",
        assetPath,
        transform);
    return EffectHandle(effect);
}

EffectHandle RenderEffectFactory::CreateOneShotParticleEffect(
    IScene* scene,
    const std::filesystem::path& assetPath,
    const EffectTransform& transform)
{
    GameObject* effect = CreateParticleObject(
        scene, "OneShotParticleEffect", assetPath, transform);
    if (effect) effect->AddComponent<OneShotParticleBehavior>();
    return EffectHandle(effect);
}

EffectHandle RenderEffectFactory::CreateMeshEffect(
    IScene* scene,
    const std::filesystem::path& assetPath,
    const EffectTransform& transform)
{
    return EffectHandle(CreateMeshEffectObject(
        scene,
        "MeshEffect",
        assetPath,
        transform));
}

AttachedEffectHandle RenderEffectFactory::CreateAttachedParticleEffect(
    IScene* scene,
    const std::filesystem::path& assetPath,
    const EffectAttachmentDesc& attachment)
{
    if (!scene || !attachment.target) return {};

    GameObject* effect = CreateParticleObject(
        scene,
        "AttachedParticleEffect",
        assetPath,
        {});
    if (!effect) return {};

    if (ParticleSystemComponent* particle = effect->GetComponent<ParticleSystemComponent>()) {
        particle->Main().playOnAwake = false;
        particle->Stop();
    }
    AttachToTransform(effect, attachment);
    AttachedEffectHandle handle{ EffectHandle(effect) };
    handle.SetLocalTransform(attachment.localTransform);
    return handle;
}

AttachedEffectHandle RenderEffectFactory::CreateAttachedMeshEffect(
    IScene* scene,
    const std::filesystem::path& assetPath,
    const EffectAttachmentDesc& attachment)
{
    if (!scene || !attachment.target) return {};

    GameObject* effect = CreateMeshEffectObject(
        scene,
        "AttachedMeshEffect",
        assetPath,
        {});
    if (!effect) return {};

    MeshEffectComponent* meshEffect = effect->GetComponent<MeshEffectComponent>();
    meshEffect->Main().playOnAwake = false;
    meshEffect->Stop();

    AttachToTransform(effect, attachment);
    AttachedEffectHandle handle{ EffectHandle(effect) };
    handle.SetLocalTransform(attachment.localTransform);
    return handle;
}
