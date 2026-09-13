//===================================================
// game.cpp [ゲームシーン制御]
// 
// Author：Miu Kitamura
// Date  ：2025/09/05
//===================================================
#include "game.h"

#include "Game/Factory/factory.h"
#include "Game/Factory/actor_factory.h"
#include "Game/Factory/environment_factory.h"
#include "Game/Factory/ui_factory.h"
#include "Game/Factory/render_effect_factory.h"
#include "Game/Factory/prefab_factory.h"
#include "Game/Factory/projectile_factory.h"

#include "Engine/Device/keyboard.h"
#include "Engine/Device/mi_fps.h"

#include "Engine/Manager/scene_manager.h"
#include "Engine/engine_service_locator.h"

#include "Engine/Component/text_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/camera_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/decal_component.h"
#include "Engine/Component/transform_component.h"

#include "Game/PresBehavior/Camera/camera_control_behavior.h"

#include "Game/ControllerBehavior/game_feedback_controller.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_world_controller.h"
#include "Game/ControllerBehavior/Wave/wave_controller_behavior.h"
#include "Game/PresBehavior/UI/Wave/wave_ui_behavior.h"

#include "Engine/Graphics/texture_repository.h"

// ゲームシーン初期化処理
void GameScene::Initialize()
{
    this->Reset();

    // シーンが使用する環境アセットを読み込む。
    // 読み込み失敗時はEnvironmentDataのデフォルト値を使用する。
    if (EnvironmentAssetLoader* loader = EngineServiceLocator::EnvironmentLoader()) {
        loader->Load(
            "asset/Environment/default.environment.json",
            GetEnvironmentAsset());
    }

    // シーン固有の初期配置物をLevelAssetから生成する。
    LoadLevel("asset/Level/stage.generated.level.json");

    // GameEffectControllerの生成
    GameObject* gameControllerObj = this->CreateGameObject();
    gameControllerObj->SetName("GameController");
    gameControllerObj->AddComponent<TransformComponent>();
    gameControllerObj->AddComponent<GameFeedbackController>();
    gameControllerObj->AddComponent<CustomPostEffectController>();
    gameControllerObj->AddComponent<EnemyAIWorldController>();
    gameControllerObj->AddComponent<WaveControllerBehavior>();
    auto* waveUiObject = this->CreateGameObject();
    waveUiObject->SetName("WaveUi");
    waveUiObject->AddComponent<WaveUiBehavior>()->Setup(
        DATA_LOADER->GetAsset<WaveUiSettingsAsset>("asset/Data/wave_ui_settings.data.json", true));

    // camera
    GameObject* camera = EnvironmentFactory::CreateCamera(this, { 0.0f,20.0f,-1.0f }, { 0.0f,0.0f,8.0f });
    EnvironmentFactory::AttachCameraControl(camera);
    camera->SetName("MainCamera");

    // light
    EnvironmentFactory::CreatePointLight(this, { 1.0f, 1.0f, 0.0f, 1.0f }, 10.0f);

    // Field
    /*GameObject* field = this->CreateGameObject();
    Factory::CreateField(field, { 0.0f, -2.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 60.0f, 1.0f, 60.0f }, { 0.2f, 0.2f, 0.2f, 1.0f });
    {
        GameObject* field2 = this->CreateGameObject();
        Factory::CreateField(field2, { -10.0f, -2.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 3.0f, 3.0f, 6.0f }, { 0.2f, 0.2f, 0.2f, 1.0f });
        GameObject* field3 = this->CreateGameObject();
        Factory::CreateField(field3, { -10.0f, -1.4f, 8.6f }, { XMConvertToRadians(18.0f), 0.0f, 0.0f }, { 3.0f, 1.0f, 3.0f }, { 0.2f, 0.2f, 0.2f, 1.0f });
    }*/

    // プレイヤープレハブ生成
    PrefabFactory::PlayerPrefab playerPrefab = PrefabFactory::CreatePlayerPrefab(this, { 0.0f,10.0f,10.0f });

    // テスト：JointGroup
   /* GameObject* jointGroup = this->CreateGameObject();
    Factory::CreateJointGroup(jointGroup, {0.0f, 0.0f, 0.0f}, {10.0f, 1.0f, 0.0f}, 0.7f);*/

    // モデルを５つ生成
   /* for (int i = 0; i < 5; i++) {
        GameObject* modelObj = this->CreateGameObject();
        Factory::CreateModel(modelObj, "asset\\Model\\bullet.fbx", { -5.0f + i * 10.5f, 0.0f, 5.0f } ,{2.0f, 2.0f, 2.0f});
    }*/

    // かかしプレハブ生成
    PrefabFactory::CreateTrainingDummyPrefab(this, { 10.0f, 7.5f, 5.0f });

    // 敵生成はWaveControllerBehaviorに集約する。
  /*  for (int i = 0; i < 5; i++) {
        PrefabFactory::CreateEnemyPrefab(this, { -5.0f + i * 5.0f, -0.5f, 25.0f });
    }*/

    // UI生成
   // GameObject* uiText = UiFactory::CreateUiText(this, u8"Mi Engine 現在制作中のゲーム");
    //UiFactory::SetupUiTransform(uiText, { 960.0f, 300.0f }, {1.0f, 1.0f});

}

// ゲームシーン終了処理
void GameScene::Finalize()
{
    std::vector<GameObject>& gameObjects = this->GetGameObjects();
    for (GameObject& obj : gameObjects) {
        obj.Destroy();
    }
    this->CollectDestroyedGameObjects();
}

// ゲームシーン更新処理
void GameScene::Update()
{
    // 破棄予約されたGameObjectの収集
    this->CollectDestroyedGameObjects();
}

// ゲームシーン描画処理
void GameScene::Draw()
{
    
}
