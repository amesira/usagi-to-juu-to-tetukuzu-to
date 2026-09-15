#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "title_controller_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/keyboard.h"
#include "Engine/engine_service_locator.h"
#include "Game/PresBehavior/UI/Title/title_ui_behavior.h"
#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"
#include "Game/PresBehavior/Camera/overview_camera_behavior.h"
#include "Game/Factory/prefab_factory.h"
#include "Engine/Component/transform_component.h"
#include "External/ImGui/imgui.h"
#include "Game/ControllerBehavior/Result/score_save_store.h"

const TitleControllerSettings::Data& TitleControllerBehavior::Settings() const
{
    static const TitleControllerSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}

void TitleControllerBehavior::Start()
{
    auto* scene = GetOwner()->GetScene();
    const auto& logo = Settings().titleLogoObject;
    GameObject* titleLogoObject = PrefabFactory::CreateModelObject(
        scene, "asset/Model/title_logo.fbx", logo.position, logo.rotation, logo.scale);
    if (titleLogoObject) {
        titleLogoObject->SetName("TitleLogoObject");
        titleLogoObject->SetRenderLayer(RenderLayer::Particle);
        m_titleLogoTransform = titleLogoObject->GetComponent<TransformComponent>();
    }
    m_settingsAssetRevision = m_settings ? static_cast<int>(m_settings->GetRevision()) : -1;
    if (auto* object = scene->GetGameObjectByName("TitleUi")) m_titleUi = object->GetComponent<TitleUiBehavior>();
    if (m_titleUi) {
        const ScoreSaveData& saveData = ScoreSaveStore::Get();
        m_titleUi->SetHighScore(saveData.hasHighScore ? saveData.highScore.totalScore : 0);
    }
    if (auto* object = scene->GetGameObjectByName("Player")) m_playerUi = object->GetComponent<PlayerUiBehavior>();
    if (auto* object = scene->GetGameObjectByName("TitleViewCamera")) m_camera = object->GetComponent<OverviewCameraBehavior>();
    if (m_playerUi) m_playerUi->SetVisible(false);
}

void TitleControllerBehavior::Update()
{
    if (m_settings && static_cast<int>(m_settings->GetRevision()) != m_settingsAssetRevision) {
        m_settingsAssetRevision = static_cast<int>(m_settings->GetRevision());
        if (m_titleLogoTransform) {
            const auto& logo = Settings().titleLogoObject;
            m_titleLogoTransform->SetPosition(logo.position);
            m_titleLogoTransform->SetEulerRawAngle(logo.rotation);
            m_titleLogoTransform->SetScaling(logo.scale);
        }
    }
    if (m_state == State::QuitRequested || m_state == State::StartingGame) return;
    if (m_state == State::ExitConfirm) {
        if (Keyboard_IsKeyDownTrigger(KK_BACK)) {
            CancelExitConfirmation();
            return;
        }
        const bool previousSelection = m_exitYesSelected;
        if (Keyboard_IsKeyDownTrigger(KK_LEFT)) m_exitYesSelected = true;
        if (Keyboard_IsKeyDownTrigger(KK_RIGHT)) m_exitYesSelected = false;
        if (m_titleUi && previousSelection != m_exitYesSelected) {
            m_titleUi->SetExitConfirmationSelection(m_exitYesSelected);
            if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuMove);
        }
        if (Keyboard_IsKeyDownTrigger(KK_ENTER)) ConfirmExitSelection();
        return;
    }
    if (m_state == State::EnteringPractice) {
        if (m_camera && m_camera->GetMode() == OverviewCameraBehavior::Mode::FollowingTps) {
            m_state = State::Practice;
            if (m_playerUi) m_playerUi->SetVisible(true);
        }
        return;
    }
    if (m_state == State::Practice) {
        // Escape currently invokes the application's native quit dialog.
        if (Keyboard_IsKeyDownTrigger(KK_BACK)) ReturnToTitle();
        return;
    }
    if (!m_titleUi) return;
    const int previousMenu = m_selectedMenu;
    if (Keyboard_IsKeyDownTrigger(KK_UP)) m_selectedMenu = (m_selectedMenu + 2) % 3;
    if (Keyboard_IsKeyDownTrigger(KK_DOWN)) m_selectedMenu = (m_selectedMenu + 1) % 3;
    if (previousMenu != m_selectedMenu) { if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuMove); }
    m_titleUi->SetSelectedMenu(static_cast<TitleUi::MenuItem>(m_selectedMenu));
    if (Keyboard_IsKeyDownTrigger(KK_ENTER)) {
        if (m_selectedMenu == 0) EnterPractice();
        else if (m_selectedMenu == 1) StartGame();
        else if (m_selectedMenu == 2) OpenExitConfirmation();
    }
}

void TitleControllerBehavior::DrawComponentInspector()
{
    ImGui::PushID(this);
    if (m_state == State::Menu) {
        if (ImGui::Button("Enter practice")) EnterPractice();
        if (ImGui::Button("Start game with fade")) StartGame();
        if (ImGui::Button("Open exit confirmation")) OpenExitConfirmation();
    }
    else if (m_state == State::ExitConfirm) {
        bool yes = m_exitYesSelected;
        if (ImGui::Checkbox("Select yes", &yes)) {
            m_exitYesSelected = yes;
            if (m_titleUi) m_titleUi->SetExitConfirmationSelection(yes);
        }
        if (ImGui::Button("Confirm selection")) ConfirmExitSelection();
        if (ImGui::Button("Cancel")) CancelExitConfirmation();
    }
    else if (m_state == State::QuitRequested) ImGui::TextUnformatted("Quit requested.");
    else if (m_state == State::StartingGame) ImGui::TextUnformatted("Fading to game...");
    else if (m_state == State::Practice) {
        ImGui::TextUnformatted("Backspace: return to title");
        if (ImGui::Button("Return to title")) ReturnToTitle();
    }
    else ImGui::TextUnformatted("Moving camera to TPS...");
    ImGui::PopID();
}

void TitleControllerBehavior::EnterPractice()
{
    if (m_state != State::Menu || !m_camera || !m_camera->BeginTpsFollow()) return;
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuConfirm);
    m_state = State::EnteringPractice;
    if (m_titleUi) {
        m_titleUi->SetVisible(false);
        m_titleUi->SetExitPopupVisible(false);
    }
    if (m_playerUi) m_playerUi->SetVisible(false);
}

void TitleControllerBehavior::ReturnToTitle()
{
    if (m_state == State::QuitRequested || m_state == State::StartingGame) return;
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuCancel);
    m_state = State::Menu;
    if (m_camera) m_camera->ReturnToOverview();
    if (m_titleUi) {
        m_titleUi->SetVisible(true);
        m_titleUi->SetExitPopupVisible(false);
    }
    if (m_playerUi) m_playerUi->SetVisible(false);
}

void TitleControllerBehavior::StartGame()
{
    if (m_state != State::Menu) return;
    if (EngineServiceLocator::ChangeSceneWithFade(SceneManager::SceneID::Game)) {
        if (auto* audio = Game::Audio()) { audio->PlaySe(GameSe::MenuConfirm); audio->StopBgm(); }
        m_state = State::StartingGame;
    }
}

void TitleControllerBehavior::OpenExitConfirmation()
{
    if (m_state != State::Menu || !m_titleUi) return;
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuConfirm);
    m_selectedMenu = 2;
    m_exitYesSelected = false;
    m_state = State::ExitConfirm;
    m_titleUi->SetSelectedMenu(TitleUi::MenuItem::Exit);
    m_titleUi->SetExitPopupVisible(true);
    m_titleUi->SetExitConfirmationSelection(false, false);
}

void TitleControllerBehavior::CancelExitConfirmation()
{
    if (m_state != State::ExitConfirm) return;
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuCancel);
    m_state = State::Menu;
    if (m_titleUi) m_titleUi->SetExitPopupVisible(false);
}

void TitleControllerBehavior::ConfirmExitSelection()
{
    if (m_state != State::ExitConfirm) return;
    if (!m_exitYesSelected) {
        CancelExitConfirmation();
        return;
    }
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuConfirm);
    m_state = State::QuitRequested;
    EngineServiceLocator::RequestQuit();
}
