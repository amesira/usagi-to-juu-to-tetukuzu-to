#include "prefab_factory.h"

#include "Engine/Asset/DataAsset/data_asset_loader.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/engine_service_locator.h"

#include "Engine/Component/collider_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/transform_component.h"

#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"
#include "Game/ActorBehavior/Base/ReactionEffects/blinker_behavior.h"
#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/ActorBehavior/Player/P40_Weapon/player_weapon_settings_asset.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_settings_asset.h"
#include "Game/ActorBehavior/TrainingDummy/training_dummy_behavior.h"

#include "Game/Factory/Prefab/player_prefab_settings_asset.h"
#include "Game/Factory/Prefab/training_dummy_prefab_settings_asset.h"
#include "Game/PresBehavior/Camera/camera_settings_asset.h"
#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"
#include "Game/PresBehavior/UI/Player/player_ui_settings_asset.h"

#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move_settings_asset.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Approach/enemy_approach_settings_asset.h"

#include "actor_factory.h"

#include <filesystem>

namespace
{
    const std::filesystem::path PLAYER_PREFAB_SETTINGS_PATH =
        "asset/Data/player_prefab_settings.data.json";
    const std::filesystem::path TRAINING_DUMMY_PREFAB_SETTINGS_PATH =
        "asset/Data/training_dummy_prefab_settings.data.json";
}

PrefabFactory::PlayerPrefab PrefabFactory::CreatePlayerPrefab(
    IScene* scene,
    const XMFLOAT3& position)
{
    PlayerPrefab prefab = {};
    if (!scene) return prefab;

    PlayerPrefabSettingsAsset* settingsAsset =
        DATA_LOADER->GetAsset<PlayerPrefabSettingsAsset>(
            PLAYER_PREFAB_SETTINGS_PATH,
            true);
    if (!settingsAsset) return prefab;

    prefab.player = ActorFactory::CreatePlayer(
        scene,
        position,
        settingsAsset->GetData());
    if (!prefab.player) return prefab;

    if (auto* ui = prefab.player->GetComponent<PlayerUiBehavior>()) {
        ui->Setup(DATA_LOADER->GetAsset<PlayerUiSettingsAsset>(
            "asset/Data/player_ui_settings.data.json", true));
    }

    // 各種設定アセットをロードして、PlayerBehaviorに設定する
    PlayerMoveSettingsAsset* moveSettingsAsset =
        DATA_LOADER->GetAsset<PlayerMoveSettingsAsset>(
            "asset/Data/player_move_settings.data.json",
            true);
    PlayerShotgunSettingsAsset* shotgunSettingsAsset =
        DATA_LOADER->GetAsset<PlayerShotgunSettingsAsset>(
            "asset/Data/player_shotgun_settings.data.json",
            true);
    PlayerDualPistolsSettingsAsset* dualPistolsSettingsAsset =
        DATA_LOADER->GetAsset<PlayerDualPistolsSettingsAsset>(
            "asset/Data/player_dual_pistols_settings.data.json",
            true);
    PlayerWeaponSettingsAsset* weaponSettingsAsset =
        DATA_LOADER->GetAsset<PlayerWeaponSettingsAsset>(
            "asset/Data/player_weapon_settings.data.json",
            true);
    CameraSettingsAsset* shotgunCameraSettingsAsset =
        DATA_LOADER->GetAsset<CameraSettingsAsset>(
            "asset/Data/camera_settings_shotgun.data.json",
            true);

    if (PlayerBehavior* playerBehavior =
            prefab.player->GetComponent<PlayerBehavior>()) {
        playerBehavior->SetupPlayerMove(moveSettingsAsset);
        playerBehavior->SetupPlayerShotgun(
            shotgunSettingsAsset,
            shotgunCameraSettingsAsset);
        playerBehavior->SetupPlayerDualPistols(
            dualPistolsSettingsAsset);
        playerBehavior->SetupPlayerWeapon(weaponSettingsAsset);
    }

    return prefab;
}

PrefabFactory::TrainingDummyPrefab PrefabFactory::CreateTrainingDummyPrefab(
    IScene* scene,
    const XMFLOAT3& position)
{
    TrainingDummyPrefab prefab = {};
    if (!scene) return prefab;

    TrainingDummyPrefabSettingsAsset* settingsAsset =
        DATA_LOADER->GetAsset<TrainingDummyPrefabSettingsAsset>(
            TRAINING_DUMMY_PREFAB_SETTINGS_PATH,
            true);
    if (!settingsAsset) return prefab;

    const TrainingDummyPrefabSettings::Data& settings =
        settingsAsset->GetData();

    GameObject* dummy = scene->CreateGameObject();
    if (!dummy) return prefab;

    // Enemyタグを付与
    dummy->SetName("TrainingDummy");
    dummy->SetTag("Enemy");
    dummy->SetCollisionLayer(CollisionLayer::Enemy);
    dummy->SetRenderLayer(RenderLayer::Enemy);

    TransformComponent* transform = dummy->AddComponent<TransformComponent>();
    CapsuleColliderComponent* collider = dummy->AddComponent<CapsuleColliderComponent>();
    RigidbodyComponent* rigidbody = dummy->AddComponent<RigidbodyComponent>();
    ModelComponent* model = dummy->AddComponent<ModelComponent>();
    AnimationComponent* animation = dummy->AddComponent<AnimationComponent>();

    HealthBehavior* health = dummy->AddComponent<HealthBehavior>();
    dummy->AddComponent<HitReceiverBehavior>();
    dummy->AddComponent<TrainingDummyBehavior>();
    dummy->AddComponent<BlinkerBehavior>();

    transform->SetPosition(position);
    transform->SetScaling(settings.scaling);

    collider->SetRadius(settings.colliderRadius);
    collider->SetHeight(settings.colliderHeight);
    collider->SetCenter(settings.colliderCenter);

    rigidbody->SetMass(settings.mass);
    rigidbody->SetFriction(settings.friction);
    rigidbody->SetGravityScale(settings.gravityScale);

    if (ModelResource* modelResource = MODEL_REPOSITORY->GetModel(settings.modelAssetPath)) {
        model->SetModelResource(modelResource);
    }

    health->SetMaxHealth(settings.maxHealth);
    health->SetHealth(settings.maxHealth);

    prefab.dummy = dummy;
    return prefab;
}

PrefabFactory::EnemyPrefab PrefabFactory::CreateEnemyPrefab(IScene* scene, const XMFLOAT3& position)
{
    GameObject* enemy = scene->CreateGameObject();

    enemy->SetName("Enemy");
    enemy->SetTag("Enemy");
    enemy->SetCollisionLayer(CollisionLayer::Enemy);
    enemy->SetRenderLayer(RenderLayer::Enemy);

    auto* transform = enemy->AddComponent<TransformComponent>();
    auto* collider = enemy->AddComponent<CapsuleColliderComponent>();
    auto* rigidbody = enemy->AddComponent<RigidbodyComponent>();

    auto* model = enemy->AddComponent<ModelComponent>();
    model->SetModelResource(MODEL_REPOSITORY->GetModel("asset/Model/enemy_a_model.fbx"));
    enemy->AddComponent<AnimationComponent>();
    enemy->AddComponent<HitReceiverBehavior>();
    enemy->AddComponent<BlinkerBehavior>();

    auto* health = enemy->AddComponent<HealthBehavior>();
    auto* behavior = enemy->AddComponent<EnemyBehavior>();

    transform->SetPosition(position);
    collider->SetRadius(1.0f);
    collider->SetHeight(1.0f);
    collider->SetCenter({ 0.0f, 0.5f, 0.0f });
    rigidbody->SetMass(1.0f);
    rigidbody->SetGravityScale(0.0f);

    behavior->SetupAiAgentSettings(
        DATA_LOADER->GetAsset<EnemyAiAgentSettingsAsset>(
            "asset/Data/enemy_ai_agent_settings.data.json",
            true));
    behavior->SetupApproachSettings(
        DATA_LOADER->GetAsset<EnemyApproachSettingsAsset>(
            "asset/Data/enemy_approach_settings.data.json", true));
    behavior->SetupMoveSettings(
        DATA_LOADER->GetAsset<EnemyMoveSettingsAsset>(
            "asset/Data/enemy_move_settings.data.json",
            true));
    behavior->SetupAttackSettings(
        DATA_LOADER->GetAsset<EnemyAttackSettingsAsset>(
            "asset/Data/enemy_attack_settings.data.json",
            true));

    return { enemy };
}
