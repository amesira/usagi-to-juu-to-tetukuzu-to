// PrefabFactory.cpp
#include "player_prefab_factory.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_base.h"

// component
#include "Engine/Component/transform_component.h"
#include "Engine/Component/decal_component.h"
#include "Engine/Component/particle_system_component.h"
#include "Engine/Component/light_component.h"
#include "Engine/Component/rect_transform_component.h"

#include "Game/ActorBehavior/transform_constraint_behavior.h"
#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_behavior.h"

#include "player_prefab_settings_asset.h"

#include "Engine/engine_service_locator.h"
#include "Engine/Asset/DataAsset/data_asset_loader.h"

#include "../actor_factory.h"
#include "../render_effect_factory.h"
#include "../projectile_factory.h"
#include "../environment_factory.h"
#include "../ui_factory.h"

#include <array>
#include <filesystem>

namespace
{
    const std::filesystem::path PLAYER_PREFAB_SETTINGS_PATH =
        "asset/Data/player_prefab_settings.data.json";
}

namespace PlayerPrefabFactory
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

    // プレイヤープレハブ生成
    PlayerPrefab CreatePlayerPrefab(IScene* scene, const XMFLOAT3& position)
    {
        PlayerPrefab prefab = {};
        if (!scene) return prefab;

        PlayerPrefabSettingsAsset* settingsAsset 
            = DATA_LOADER->GetAsset<PlayerPrefabSettingsAsset>(PLAYER_PREFAB_SETTINGS_PATH, true);
        if (!settingsAsset) return prefab;

        const PlayerPrefabSettings::Data& settings = settingsAsset->GetData();

        // プレイヤー生成
        prefab.player = ActorFactory::CreatePlayer(scene, position, settings);
        if (!prefab.player) return prefab;

        TransformComponent* playerTransform = prefab.player->GetComponent<TransformComponent>();
        PlayerMoveBehavior* playerMoveBehavior = prefab.player->GetComponent<PlayerMoveBehavior>();

        PlayerMoveReferences moveReferences;
        PlayerMoveSettingsAsset* moveSettingsAsset = DATA_LOADER->GetAsset<PlayerMoveSettingsAsset>(
            "asset/Data/player_move_settings.data.json",
            true);

        PlayerShotgunReferences shotgunReferences;
        PlayerShotgunSettingsAsset* shotgunSettingsAsset = DATA_LOADER->GetAsset<PlayerShotgunSettingsAsset>(
            "asset/Data/player_shotgun_settings.data.json",
            true);

        // 走行時の砂埃パーティクル生成
        EffectHandle runDustEffectHandle = RenderEffectFactory::CreateAttachedParticleEffect(
            scene,
            playerTransform,
            "asset/Particle/run_dust.particle.json",
            settings.runDustEffect.positionOffset);
        prefab.runDustParticle = runDustEffectHandle.GetGameObject();
        moveReferences.runDustParticle = runDustEffectHandle.GetParticleSystem();
        {
            moveReferences.runDustParticle->Main().playOnAwake = true;
        }

        PlayerBehavior* playerBehavior = prefab.player->GetComponent<PlayerBehavior>();
        if (playerBehavior)
        {
            playerBehavior->SetupPlayerMove(moveReferences, moveSettingsAsset);
            playerBehavior->SetupPlayerShotgun(shotgunReferences, shotgunSettingsAsset);
        }
        return prefab;
    }
}
