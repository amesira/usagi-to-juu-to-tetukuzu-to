#include "title.h"

#include "Engine/Core/game_object.h"

#include "Engine/engine_service_locator.h"

#include "Game/ControllerBehavior/title_controller_behavior.h"
#include "Game/PresBehavior/UI/Title/title_ui_behavior.h"
#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"
#include "Game/PresBehavior/Camera/overview_camera_behavior.h"
#include "Engine/Component/camera_component.h"
#include "Game/ControllerBehavior/StageDecoration/stage_decoration_controller_behavior.h"
#include "Game/ControllerBehavior/StageBounds/stage_bounds_controller_behavior.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"

#include "Game/ControllerBehavior/game_controller_locator.h"

#include "Game/Factory/environment_factory.h"
#include "Game/Factory/prefab_factory.h"

void TitleScene::Initialize()
{
    Reset();

    // シーンが使用する環境アセットを読み込む。
    // 読み込み失敗時はEnvironmentDataのデフォルト値を使用する。
    if (EnvironmentAssetLoader* loader = EngineServiceLocator::EnvironmentLoader()) {
        loader->Load(
            "asset/Environment/default.environment.json",
            GetEnvironmentAsset());
    }

    // シーン固有の初期配置物をLevelAssetから生成する。
    LoadLevel("asset/Level/stage.generated.level.json");

    GameObject* controller = CreateGameObject();
    controller->SetName("TitleController");
    controller->AddComponent<TitleControllerBehavior>();
    GameObject* titleUi = CreateGameObject();
    titleUi->SetName("TitleUi");
    titleUi->AddComponent<TitleUiBehavior>()->Setup(
        DATA_LOADER->GetAsset<TitleUiSettingsAsset>("asset/Data/title_ui_settings.data.json", true));
    controller->AddComponent<GameAudioControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<GameAudioSettingsAsset>("asset/Data/game_audio_settings.data.json", true), GameBgm::Title);
    controller->AddComponent<GameFeedbackController>();
    controller->AddComponent<CustomPostEffectController>();
    controller->AddComponent<StageBoundsControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<StageBoundsSettingsAsset>("asset/Data/stage_bounds_settings.data.json", true));
    controller->AddComponent<StageDecorationControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<StageDecorationSettingsAsset>("asset/Data/title_decoration_settings.data.json", true));

    // camera
    GameObject* camera = EnvironmentFactory::CreateCamera(this, { 0.0f,20.0f,-1.0f }, { 0.0f,0.0f,8.0f });
    EnvironmentFactory::AttachCameraControl(camera);
    camera->SetName("MainCamera");
    camera->GetComponent<CameraComponent>()->SetRenderEnabled(false);

    GameObject* viewCamera = EnvironmentFactory::CreateCamera(this, {0,45,-25}, {0,0,10});
    viewCamera->SetName("TitleViewCamera");
    viewCamera->AddComponent<OverviewCameraBehavior>()->Setup(
        DATA_LOADER->GetAsset<OverviewCameraSettingsAsset>("asset/Data/title_camera_settings.data.json", true));

    // プレイヤープレハブ生成
    PrefabFactory::PlayerPrefab playerPrefab = PrefabFactory::CreatePlayerPrefab(this, { 0.0f,10.0f,10.0f });
    if (playerPrefab.player) {
        if (auto* ui = playerPrefab.player->GetComponent<PlayerUiBehavior>()) ui->SetVisible(false);
    }

    // かかしプレハブ生成
    PrefabFactory::CreateTrainingDummyPrefab(this, { 10.0f, 7.5f, 5.0f });

    // BGM再生
    if (auto* audioController = Game::Audio()) {
        audioController->SetBgm(GameBgm::Title);
    }

}

void TitleScene::Finalize()
{
    if (auto* audioController = Game::Audio()) {
        audioController->StopBgm();
    }

    for (GameObject& object : GetGameObjects()) {
        object.Destroy();
    }
    CollectDestroyedGameObjects();
}

void TitleScene::Update()
{
    CollectDestroyedGameObjects();
}

void TitleScene::Draw()
{
}
