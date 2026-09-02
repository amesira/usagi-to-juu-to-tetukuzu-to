#include "prefab_factory.h"

#include "Engine/Asset/DataAsset/data_asset_loader.h"
#include "Engine/Core/game_object.h"
#include "Engine/engine_service_locator.h"

#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_settings_asset.h"
#include "Game/Factory/Prefab/player_prefab_settings_asset.h"
#include "Game/PresBehavior/Camera/camera_settings_asset.h"

#include "actor_factory.h"

#include <filesystem>

namespace
{
    const std::filesystem::path PLAYER_PREFAB_SETTINGS_PATH =
        "asset/Data/player_prefab_settings.data.json";
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
    }

    return prefab;
}
