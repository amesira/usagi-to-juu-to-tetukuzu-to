//===================================================
// projectile_factory.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/06/07
//===================================================
#include "projectile_factory.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/model_component.h"
#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/text_component.h"

#include "Game/ActorBehavior/Bullet/bullet_behavior.h"
#include "Game/ActorBehavior/Base/ReactionEffects/blinker_behavior.h"
#include "Game/PresBehavior/UI/damage_number_behavior.h"

#include "Engine/Graphics/material_repository.h"
#include "Engine/Graphics/model_repository.h"
#include "Engine/Graphics/shader_repository.h"
#include "Engine/Graphics/texture_repository.h"

#include "Engine/engine_service_locator.h"
#include "Utility/mi_math.h"

#include "Game/Factory/render_effect_factory.h"

namespace
{
    const std::filesystem::path BULLET_PARTICLE_PATH = "asset//Particle//bullet_trail.particle.json";

    // ホログラムシェーダーの取得または生成
    ShaderProgramResource* GetOrCreateHologramShader(const ProjectileFactory::BulletCreateDesc& desc)
    {
        if (!SHADER_REPOSITORY) return nullptr;
        const char* hologramShaderName = "HologramUnlit";
        const char* hologramPixelShaderPath = "hologram_unlit_ps.cso";

        if (ShaderProgramResource* shader = SHADER_REPOSITORY->GetShaderProgramResource(hologramShaderName)) {
            return shader;
        }

        // シェーダーリソースの生成
        ShaderProgramResource shaderResource = {};
        shaderResource.name = hologramShaderName;
        shaderResource.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::Unlit);
        shaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource(hologramPixelShaderPath);

        if (!shaderResource.baseShader || !shaderResource.overridePixelShader) return nullptr;

        return SHADER_REPOSITORY->GenerateShaderProgramResource(shaderResource);
    }

    // 弾のマテリアルの生成
    MaterialResource* CreateBulletMaterial(const ProjectileFactory::BulletCreateDesc& desc)
    {
        if (!MATERIAL_REPOSITORY) return nullptr;
        static bool generated = false;

        if (generated) {
            MaterialResource* existingMaterial = MATERIAL_REPOSITORY->GetMaterial(desc.materialName);
            if (existingMaterial) {
                return existingMaterial;
            }
        }
        generated = true;
        
        XMFLOAT4 hologramColor = { 0.85f, 0.55f, 1.0f, 0.75f };
        float hologramIntensity = 7.0f;

        MaterialResource material = {};
        material.name = desc.materialName;
        material.renderMode = RenderMode::Opaque;
        material.shaderProgram = GetOrCreateHologramShader(desc);
        material.baseColor = hologramColor;
        material.emissiveColor = { hologramColor.x, hologramColor.y, hologramColor.z };
        material.emissiveIntensity = hologramIntensity;

        material.customProperties[0] = hologramColor;
        material.customProperties[1] = { hologramIntensity, 0.0f, 0.0f, 0.0f };

        if (TEXTURE_REPOSITORY) {
            material.customTextures[0] = TEXTURE_REPOSITORY->GetTextureResource(L"asset\\Texture\\hologram_noise.png");
            material.customTextures[1] = TEXTURE_REPOSITORY->GetTextureResource(L"asset\\Texture\\Dither.png");
        }

        return MATERIAL_REPOSITORY->GenerateMaterial(material);
    }

    // モデルコンポーネントのセットアップ
    void SetupBulletModel(ModelComponent* modelComponent, const ProjectileFactory::BulletCreateDesc& desc)
    {
        if (!modelComponent || !MODEL_REPOSITORY) return;

        ModelResource* modelResource = MODEL_REPOSITORY->GetModel(desc.modelPath);
        if (!modelResource) return;
        modelComponent->SetModelResource(modelResource);

        // ホログラムマテリアルの生成と適用
        MaterialResource* bulletMaterial = CreateBulletMaterial(desc);
        if (!bulletMaterial) return;

        // モデルの全マテリアルスロットにホログラムマテリアルを適用
        for (MaterialInstance& materialSlot : modelComponent->GetMaterialSlots()) {
            materialSlot.materialResource = bulletMaterial;
        }
    }
}
// 弾の生成
GameObject* ProjectileFactory::CreateBullet(IScene* scene, const BulletCreateDesc& desc)
{
    if (!scene) return nullptr;

    GameObject* bullet = scene->CreateGameObject();
    bullet->SetName("Bullet");
    bullet->SetRenderLayer(RenderLayer::Bullet);
    if (!bullet) return nullptr;

    TransformComponent* transform = bullet->AddComponent<TransformComponent>();
    if (!transform) return nullptr;

    ModelComponent* modelComponent = bullet->AddComponent<ModelComponent>();
    ParticleSystemComponent* particleSystem = bullet->AddComponent<ParticleSystemComponent>();

    BulletBehavior* bulletBehavior = bullet->AddComponent<BulletBehavior>();
    
    bullet->AddComponent<BlinkerBehavior>();

    transform->SetPosition(desc.position);
    transform->SetScaling({ desc.radius * 2.5f, desc.radius * 2.5f, desc.radius * 2.5f });

    // 弾の進行方向に合わせて回転を設定
    XMFLOAT3 forward = MiMath::Multiply(desc.velocity, -1.0f);
    MiMath::Normalize(forward);
    XMFLOAT4 rotation = MiMath::QuaternionFromDirection(forward, { 0.0f, 1.0f, 0.0f });
    transform->SetRotation(rotation);

    SetupBulletModel(modelComponent, desc);

    if (!RenderEffectFactory::ApplyParticleAsset(particleSystem, BULLET_PARTICLE_PATH)) {
        EngineServiceLocator::AddLogMessage(
            "Failed to load particle asset: " + BULLET_PARTICLE_PATH.generic_string());
    }

    bulletBehavior->Initialize(
        desc.velocity,
        desc.radius,
        desc.lifeTime,
        desc.layerMask,
        desc.attacker,
        desc.damage,
        desc.attackType);

    return bullet;
}

GameObject* ProjectileFactory::CreateDamageNumber(IScene* scene, const DamageNumberCreateDesc& desc)
{
    GameObject* damageNumber = scene->CreateGameObject();
    damageNumber->SetName("DamageNumber");
    damageNumber->SetRenderLayer(RenderLayer::Default);

    TransformComponent* transform = damageNumber->AddComponent<TransformComponent>();
    transform->SetPosition(desc.position);
    
    TextComponent* textComponent = damageNumber->AddComponent<TextComponent>();
    textComponent->SetCenter(true);
    textComponent->SetFontPath(desc.fontPath);
    textComponent->SetFontSize(36);

    DamageNumberBehavior* behavior = damageNumber->AddComponent<DamageNumberBehavior>();
    behavior->Show(desc.damage, desc.position, desc.color);

    return damageNumber;
}
