#include "result.h"

#include "Engine/Core/game_object.h"

#include "Engine/engine_service_locator.h"

#include "Game/ControllerBehavior/result_controller_behavior.h"
#include "Game/PresBehavior/UI/Result/result_ui_behavior.h"
#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"
#include "Game/PresBehavior/Camera/overview_camera_behavior.h"
#include "Engine/Component/camera_component.h"
#include "Game/ControllerBehavior/StageDecoration/stage_decoration_controller_behavior.h"
#include "Game/ControllerBehavior/StageBounds/stage_bounds_controller_behavior.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Game/ControllerBehavior/ScoreboardEnemy/scoreboard_enemy_controller_behavior.h"
#include "Game/ControllerBehavior/ScoreboardEnemy/scoreboard_enemy_settings_asset.h"

#include "Game/ControllerBehavior/game_controller_locator.h"

#include "Game/Factory/environment_factory.h"
#include "Game/Factory/prefab_factory.h"

void ResultScene::Initialize()
{
    Reset();

    // シーンが使用する環境アセットを読み込む。
    // 読み込み失敗時はEnvironmentDataのデフォルト値を使用する。
    if (EnvironmentAssetLoader* loader = EngineServiceLocator::EnvironmentLoader()) {
        loader->Load(
            "asset/Environment/result.environment.json",
            GetEnvironmentAsset());
    }

    // シーン固有の初期配置物をLevelAssetから生成する。
    LoadLevel("asset/Level/stage.generated.level.json");

    GameObject* controller = CreateGameObject();
    controller->SetName("ResultController");
    controller->AddComponent<ResultControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<ResultControllerSettingsAsset>("asset/Data/result_controller_settings.data.json", true));
    GameObject* resultUi = CreateGameObject();
    resultUi->SetName("ResultUi");
    resultUi->AddComponent<ResultUiBehavior>()->Setup(
        DATA_LOADER->GetAsset<ResultUiSettingsAsset>("asset/Data/result_ui_settings.data.json", true));
    controller->AddComponent<GameAudioControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<GameAudioSettingsAsset>("asset/Data/game_audio_settings.data.json", true), GameBgm::Result);
    controller->AddComponent<GameFeedbackController>();
    controller->AddComponent<CustomPostEffectController>();
    controller->AddComponent<EnemyAIWorldController>()->Setup(false);
    controller->AddComponent<ScoreboardEnemyControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<ScoreboardEnemySettingsAsset>("asset/Data/scoreboard_enemy_settings.data.json", true));
    controller->AddComponent<StageBoundsControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<StageBoundsSettingsAsset>("asset/Data/stage_bounds_settings.data.json", true));
    controller->AddComponent<StageDecorationControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<StageDecorationSettingsAsset>("asset/Data/stage_decoration_settings.data.json", true));



    // camera
    GameObject* camera = EnvironmentFactory::CreateCamera(this, { 0.0f,20.0f,-1.0f }, { 0.0f,0.0f,8.0f });
    EnvironmentFactory::AttachCameraControl(camera);
    camera->SetName("MainCamera");
    camera->GetComponent<CameraComponent>()->SetRenderEnabled(false);

    GameObject* viewCamera = EnvironmentFactory::CreateCamera(this, {0,45,-25}, {0,0,10});
    viewCamera->SetName("ResultViewCamera");
    viewCamera->AddComponent<OverviewCameraBehavior>()->Setup(
        DATA_LOADER->GetAsset<OverviewCameraSettingsAsset>("asset/Data/result_camera_settings.data.json", true));

    // プレイヤープレハブ生成
    PrefabFactory::PlayerPrefab playerPrefab = PrefabFactory::CreatePlayerPrefab(this, { 0.0f,10.0f,10.0f });
    if (playerPrefab.player) {
        if (auto* ui = playerPrefab.player->GetComponent<PlayerUiBehavior>()) ui->SetVisible(false);
    }

    // BGM再生
    if (auto* audioController = Game::Audio()) {
        audioController->SetBgm(GameBgm::Result);
    }

}

void ResultScene::Finalize()
{
    if (auto* audioController = Game::Audio()) {
        audioController->StopBgm();
    }

    for (GameObject& object : GetGameObjects()) {
        object.Destroy();
    }
    CollectDestroyedGameObjects();
}

void ResultScene::Update()
{
    CollectDestroyedGameObjects();
}

void ResultScene::Draw()
{
}
