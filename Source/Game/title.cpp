#include "title.h"

#include "Engine/Core/game_object.h"

#include "Engine/engine_service_locator.h"

#include "Game/ControllerBehavior/title_controller_behavior.h"
#include "Game/ControllerBehavior/StageDecoration/stage_decoration_controller_behavior.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"

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
    controller->AddComponent<GameFeedbackController>();
    controller->AddComponent<CustomPostEffectController>();
    controller->AddComponent<StageDecorationControllerBehavior>()->Setup(
        DATA_LOADER->GetAsset<StageDecorationSettingsAsset>("asset/Data/stage_decoration_settings.data.json", true));

    // camera
    GameObject* camera = EnvironmentFactory::CreateCamera(this, { 0.0f,20.0f,-1.0f }, { 0.0f,0.0f,8.0f });
    EnvironmentFactory::AttachCameraControl(camera);
    camera->SetName("MainCamera");

    // プレイヤープレハブ生成
    PrefabFactory::PlayerPrefab playerPrefab = PrefabFactory::CreatePlayerPrefab(this, { 0.0f,10.0f,10.0f });

    // かかしプレハブ生成
    PrefabFactory::CreateTrainingDummyPrefab(this, { 10.0f, 7.5f, 5.0f });

}

void TitleScene::Finalize()
{
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
