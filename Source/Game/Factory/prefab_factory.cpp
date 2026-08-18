// PrefabFactory.cpp
#include "prefab_factory.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_base.h"

// component
#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/decal_component.h"
#include "Engine/Framework/Component/particle_system_component.h"
#include "Engine/Framework/Component/light_component.h"
#include "Engine/Framework/Component/rect_transform_component.h"

#include "Game/ActorBehavior/transform_constraint_behavior.h"
#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/ActorBehavior/Player/player_prefab_settings_asset.h"
#include "Game/ActorBehavior/Player/P30_Action/Attack/player_attack_behavior.h"
#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_behavior.h"

#include "Engine/engine_service_locator.h"
#include "Engine/Asset/DataAsset/data_asset_loader.h"

#include "actor_factory.h"
#include "render_effect_factory.h"
#include "projectile_factory.h"
#include "environment_factory.h"
#include "ui_factory.h"

#include <array>
#include <filesystem>

namespace
{
    const std::filesystem::path PLAYER_PREFAB_SETTINGS_PATH =
        "asset/Data/player_prefab_settings.data.json";
}

namespace PrefabFactory
{
    // TransformConstraintBehaviorのセットアップ
    void SetupTransformConstraint(GameObject* element, 
        TransformComponent* target, const XMFLOAT3& positionOffset, 
        bool considerRotation = true, bool considerScale = false) 
    {
        TransformConstraintBehavior* constraint = element->AddComponent<TransformConstraintBehavior>();
        constraint->SetTarget(target);
        constraint->SetOffset(positionOffset);
        constraint->SetConsiderRotation(considerRotation);
        constraint->SetConsiderScaling(considerScale);
    }

    // ----------------------------------- プレハブ生成

    // プレイヤープレハブ生成
    PlayerPrefab PrefabFactory::CreatePlayerPrefab(IScene* scene, const XMFLOAT3& position)
    {
        PlayerPrefab prefab{};
        if (!scene) return prefab;

        PlayerPrefabSettingsAsset* settingsAsset = DATA_LOADER->GetAsset<PlayerPrefabSettingsAsset>(
                PLAYER_PREFAB_SETTINGS_PATH,
                true);
        if (!settingsAsset) return prefab;

        const PlayerPrefabSettings::Data& settings = settingsAsset->GetData();

        // プレイヤー生成
        prefab.player = ActorFactory::CreatePlayer(scene, position, settings);
        if (!prefab.player) return prefab;

        TransformComponent* playerTransform = prefab.player->GetComponent<TransformComponent>();
        PlayerAttackBehavior* playerAttackBehavior = prefab.player->GetComponent<PlayerAttackBehavior>();
        PlayerMoveBehavior* playerMoveBehavior = prefab.player->GetComponent<PlayerMoveBehavior>();

        // 走行時の砂埃パーティクル生成
        prefab.runDustParticle = RenderEffectFactory::CreateRunDustParticle(
            scene,
            prefab.player,
            settings.runDustEffect.particleAssetPath);
        if (prefab.runDustParticle)
        {
            SetupTransformConstraint(
                prefab.runDustParticle,
                playerTransform,
                settings.runDustEffect.positionOffset,
                true,
                false);
            playerMoveBehavior->SetupRunDustParticle(prefab.runDustParticle->GetComponent<ParticleSystemComponent>());
        }
        // チャージ時の吸収パーティクル生成
        prefab.chargeEffectParticle = RenderEffectFactory::CreateChargeAbsorbParticle(scene, position);
        {
            SetupTransformConstraint(prefab.chargeEffectParticle, playerTransform, { 1.0f, -0.3f, 0.0f }, true, false);
            ParticleSystemComponent* particleSystem = prefab.chargeEffectParticle->GetComponent<ParticleSystemComponent>();
            particleSystem->Main().playOnAwake = false; // 最初は再生しない
            playerAttackBehavior->SetupChargeEffect(particleSystem);
        }
        // チャージ時のライト生成
        prefab.chargeLight = EnvironmentFactory::CreatePointLight(scene, { 1.0f, 0.5f, 0.0f, 1.0f }, 3.0f);
        {
            SetupTransformConstraint(prefab.chargeLight, playerTransform, { 1.0f, -0.3f, -0.5f }, true, false);
            LightComponent* lightComp = prefab.chargeLight->GetComponent<LightComponent>();
            lightComp->SetEnable(false); // 最初はライトをオフにする
            playerAttackBehavior->SetupChargeLight(lightComp);
        }
        return prefab;
    }
}
