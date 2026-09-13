#include "title.h"

#include "Engine/Core/game_object.h"
#include "Game/ControllerBehavior/title_controller_behavior.h"

void TitleScene::Initialize()
{
    Reset();

    GameObject* controller = CreateGameObject();
    controller->SetName("TitleController");
    controller->AddComponent<TitleControllerBehavior>();
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
