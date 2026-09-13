#include "title_controller_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/keyboard.h"
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
    if (Keyboard_IsKeyDownTrigger(KK_ENTER) && m_selectedMenu == 0) EnterPractice();
}

void TitleControllerBehavior::DrawComponentInspector()
{
    ImGui::PushID(this);
    if (m_state == State::Menu) {
        if (ImGui::Button("Enter practice")) EnterPractice();
    }
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
    m_state = State::Menu;
    if (m_camera) m_camera->ReturnToTitle();
    if (m_titleUi) m_titleUi->SetVisible(true);
    if (m_playerUi) m_playerUi->SetVisible(false);
}
