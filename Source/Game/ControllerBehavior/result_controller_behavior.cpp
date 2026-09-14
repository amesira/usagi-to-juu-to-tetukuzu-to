#include "result_controller_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/transform_component.h"

#include "Game/Factory/prefab_factory.h"
#include "Game/PresBehavior/UI/Result/result_ui_behavior.h"
#include "Engine/Device/keyboard.h"
#include "Engine/engine_service_locator.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"

const ResultControllerSettings::Data& ResultControllerBehavior::Settings() const
{
    static const ResultControllerSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}

void ResultControllerBehavior::Start()
{
    // ResultTextObjectを生成して配置
    GameObject* resultTextObject = PrefabFactory::CreateModelObject(
        GetOwner()->GetScene(),
        "asset/Model/result_logo.fbx",
        Settings().resultTextObject.position,
        Settings().resultTextObject.rotation,
        Settings().resultTextObject.scale);
    if (resultTextObject) {
        resultTextObject->SetName("ResultTextObject");
        resultTextObject->SetRenderLayer(RenderLayer::Particle);
    }
    m_resultTextTransform = resultTextObject ? resultTextObject->GetComponent<TransformComponent>() : nullptr;

    m_settingsAssetRivision = m_settings ? m_settings->GetRevision() : -1;
    if (auto* object = GetOwner()->GetScene()->GetGameObjectByName("ResultUi")) {
        m_resultUi = object->GetComponent<ResultUiBehavior>();
        if (m_resultUi) m_resultUi->SetResult(GameResultStore::lastRun, ResultScoring::Calculate(GameResultStore::lastRun, Settings().scoring));
    }
}

void ResultControllerBehavior::Update()
{
    if (m_settings && m_settings->GetRevision() != m_settingsAssetRivision) {
        m_settingsAssetRivision = m_settings->GetRevision();

        if (m_resultTextTransform) {
            m_resultTextTransform->SetPosition(Settings().resultTextObject.position);
            m_resultTextTransform->SetEulerRawAngle(Settings().resultTextObject.rotation);
            m_resultTextTransform->SetScaling(Settings().resultTextObject.scale);
        }
    }
    if (m_leaving || !m_resultUi || !m_resultUi->IsMenuReady()) return;
    int selection = m_resultUi->GetSelection();
    const int previousSelection = selection;
    if (Keyboard_IsKeyDownTrigger(KK_LEFT)) selection = 0;
    if (Keyboard_IsKeyDownTrigger(KK_RIGHT)) selection = 1;
    if (selection != previousSelection) {
        m_resultUi->SetSelection(selection);
        if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuMove);
    }
    if (Keyboard_IsKeyDownTrigger(KK_ENTER)
        && EngineServiceLocator::ChangeSceneWithFade(selection == 0 ? SceneManager::SceneID::Title : SceneManager::SceneID::Game)) {
        m_leaving = true;
        GameResultStore::lastRun = {};
        if (auto* audio = Game::Audio()) { audio->PlaySe(GameSe::MenuConfirm); audio->StopBgm(); }
    }
}

void ResultControllerBehavior::DrawComponentInspector()
{

}
