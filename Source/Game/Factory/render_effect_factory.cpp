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
#include "Engine/Framework/Component/decal_component.h"
#include "Engine/Framework/Component/particle_system_component.h"
#include "Engine/Framework/Component/transform_component.h"
#include "Engine/engine_service_locator.h"
#include "Utility/mi_string.h"

namespace
{
    const std::filesystem::path GENERIC_PARTICLE_ASSET = "asset/Particle/generic_particle.json";
    const std::filesystem::path RUN_DUST_PARTICLE_ASSET = "asset/Particle/run_dust.json";
    const std::filesystem::path CHARGE_ABSORB_PARTICLE_ASSET = "asset/Particle/charge_absorb.json";
    const std::filesystem::path HIT_PARTICLE_ASSET = "asset/Particle/hit.json";
    const std::filesystem::path EXPLOSION_PARTICLE_ASSET = "asset/Particle/explosion.json";

    bool ApplyParticleAsset(
        ParticleSystemComponent* particleSystem,
        const std::filesystem::path& assetPath,
        const std::wstring* textureOverride = nullptr)
    {
        if (!particleSystem) return false;

        ParticleSystemAssetLoader* loader = EngineServiceLocator::GetParticleAssetLoader();
        if (!loader) return false;

        ParticleSystemAsset* asset = loader->GetParticle(assetPath);
        if (!asset) return false;

        particleSystem->GetDesc() = asset->GetDesc();

        auto& renderer = particleSystem->Renderer();
        TextureResource* texture = nullptr;
        TextureRepository* textureRepository = EngineServiceLocator::GetTextureRepository();
        if (textureRepository)
        {
            if (textureOverride)
            {
                texture = textureRepository->GetTextureResource(*textureOverride);
            }
            else if (!renderer.texturePath.empty())
            {
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
}

GameObject* RenderEffectFactory::CreateDecalEffect(
    SceneBase* scene,
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

GameObject* RenderEffectFactory::CreateParticleEffect(
    SceneBase* scene,
    const XMFLOAT3& position,
    const std::wstring& texturePath)
{
    return CreateParticleObject(
        scene,
        "ParticleEffect",
        position,
        GENERIC_PARTICLE_ASSET,
        &texturePath);
}

GameObject* RenderEffectFactory::CreateRunDustParticle(SceneBase* scene, std::string targetName)
{
    if (!scene) return nullptr;

    GameObject* target = scene->GetGameObjectByName(targetName);
    if (!target) return nullptr;

    TransformComponent* targetTransform = target->GetComponent<TransformComponent>();
    if (!targetTransform) return nullptr;

    return CreateParticleObject(
        scene,
        "RunDustParticle",
        { 0.0f, 0.0f, 0.0f },
        RUN_DUST_PARTICLE_ASSET);
}

GameObject* RenderEffectFactory::CreateChargeAbsorbParticle(
    SceneBase* scene,
    const XMFLOAT3& position)
{
    return CreateParticleObject(
        scene,
        "ChargeAbsorbParticle",
        position,
        CHARGE_ABSORB_PARTICLE_ASSET);
}

GameObject* RenderEffectFactory::CreateHitEffect(IScene* scene, const XMFLOAT3& position)
{
    return CreateParticleObject(scene, "HitEffect", position, HIT_PARTICLE_ASSET);
}

GameObject* RenderEffectFactory::CreateExplosionEffect(IScene* scene, const XMFLOAT3& position)
{
    return CreateParticleObject(scene, "ExplosionEffect", position, EXPLOSION_PARTICLE_ASSET);
}
