#include "title_controller_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/keyboard.h"
#include "Engine/engine_service_locator.h"
#include "Game/PresBehavior/UI/Title/title_ui_behavior.h"
#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"
#include "Game/PresBehavior/Camera/title_camera_behavior.h"
#include "External/ImGui/imgui.h"

void TitleControllerBehavior::Start()
{
    auto* scene = GetOwner()->GetScene();
    if (auto* object = scene->GetGameObjectByName("TitleUi")) m_titleUi = object->GetComponent<TitleUiBehavior>();
    if (auto* object = scene->GetGameObjectByName("Player")) m_playerUi = object->GetComponent<PlayerUiBehavior>();
    if (auto* object = scene->GetGameObjectByName("TitleViewCamera")) m_camera = object->GetComponent<TitleCameraBehavior>();
    if (m_playerUi) m_playerUi->SetVisible(false);
}

void TitleControllerBehavior::Update()
{
    if (m_state == State::QuitRequested) return;
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
        }
        if (Keyboard_IsKeyDownTrigger(KK_ENTER)) ConfirmExitSelection();
        return;
    }
    if (m_state == State::EnteringPractice) {
        if (m_camera && m_camera->GetMode() == TitleCameraBehavior::Mode::FollowingTps) {
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
    if (Keyboard_IsKeyDownTrigger(KK_UP)) m_selectedMenu = (m_selectedMenu + 2) % 3;
    if (Keyboard_IsKeyDownTrigger(KK_DOWN)) m_selectedMenu = (m_selectedMenu + 1) % 3;
    m_titleUi->SetSelectedMenu(static_cast<TitleUi::MenuItem>(m_selectedMenu));
    if (Keyboard_IsKeyDownTrigger(KK_ENTER)) {
        if (m_selectedMenu == 0) EnterPractice();
        else if (m_selectedMenu == 2) OpenExitConfirmation();
    }
}

void TitleControllerBehavior::DrawComponentInspector()
{
    ImGui::PushID(this);
    if (m_state == State::Menu) {
        if (ImGui::Button("Enter practice")) EnterPractice();
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
    else if (m_state == State::Practice) {
        ImGui::TextUnformatted("Backspace: return to title");
        if (ImGui::Button("Return to title")) ReturnToTitle();
    }
    else ImGui::TextUnformatted("Moving camera to TPS...");
    ImGui::PopID();
}

void TitleControllerBehavior::EnterPractice()
{
    if (m_state != State::Menu || !m_camera || !m_camera->BeginPractice()) return;
    m_state = State::EnteringPractice;
    if (m_titleUi) {
        m_titleUi->SetVisible(false);
        m_titleUi->SetExitPopupVisible(false);
    }
    if (m_playerUi) m_playerUi->SetVisible(false);
}

void TitleControllerBehavior::ReturnToTitle()
{
    if (m_state == State::QuitRequested) return;
    m_state = State::Menu;
    if (m_camera) m_camera->ReturnToTitle();
    if (m_titleUi) {
        m_titleUi->SetVisible(true);
        m_titleUi->SetExitPopupVisible(false);
    }
    if (m_playerUi) m_playerUi->SetVisible(false);
}

void TitleControllerBehavior::OpenExitConfirmation()
{
    if (m_state != State::Menu || !m_titleUi) return;
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
    m_state = State::QuitRequested;
    EngineServiceLocator::RequestQuit();
}
