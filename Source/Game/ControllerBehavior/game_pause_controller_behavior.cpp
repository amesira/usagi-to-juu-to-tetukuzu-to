#include "game_pause_controller_behavior.h"
#include "Game/PresBehavior/UI/GamePause/game_pause_ui_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/Wave/wave_controller_behavior.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/keyboard.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/engine_service_locator.h"
#include "Engine/Manager/scene_manager.h"
#include "External/ImGui/imgui.h"

GamePauseControllerBehavior::~GamePauseControllerBehavior() {
    if (m_open && !m_changingScene) FPS_SetTimeScale(m_previousTimeScale);
    if (GameControllerLocator::s_pauseController == this) GameControllerLocator::s_pauseController = nullptr;
}

void GamePauseControllerBehavior::Start() {
    if (!GetOwner() || !GetOwner()->GetScene()) return;
    GameControllerLocator::s_pauseController = this;
    if (auto* object = GetOwner()->GetScene()->GetGameObjectByName("GamePauseUi"))
        m_ui = object->GetComponent<GamePauseUiBehavior>();
    if (m_ui) { m_ui->SetSelection(false, false); m_ui->SetVisible(false); }
}

bool GamePauseControllerBehavior::CanOpen() const {
    auto* wave = Game::Wave();
    if (!wave) return false;
    const auto state = wave->GetProgress().state;
    return state != WaveProgress::State::ClearImpact
        && state != WaveProgress::State::Complete
        && state != WaveProgress::State::GameOver;
}

void GamePauseControllerBehavior::Open() {
    if (m_open || !m_ui || !CanOpen()) return;
    m_previousTimeScale = FPS_GetTimeScale();
    m_open = true; m_yesSelected = false;
    FPS_SetTimeScale(0.0f);
    m_ui->SetSelection(false, false); m_ui->SetVisible(true);
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuConfirm);
}

void GamePauseControllerBehavior::Close() {
    if (!m_open) return;
    m_open = false;
    FPS_SetTimeScale(m_previousTimeScale);
    if (m_ui) m_ui->SetVisible(false);
    if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuCancel);
}

void GamePauseControllerBehavior::Update() {
    if (m_changingScene) return;
    if (!m_open) { if (Keyboard_IsKeyDownTrigger(KK_BACK)) Open(); return; }
    if (Keyboard_IsKeyDownTrigger(KK_BACK)) { Close(); return; }
    const bool previous = m_yesSelected;
    if (Keyboard_IsKeyDownTrigger(KK_LEFT)) m_yesSelected = true;
    if (Keyboard_IsKeyDownTrigger(KK_RIGHT)) m_yesSelected = false;
    if (previous != m_yesSelected) {
        if (m_ui) m_ui->SetSelection(m_yesSelected);
        if (auto* audio = Game::Audio()) audio->PlaySe(GameSe::MenuMove);
    }
    if (!Keyboard_IsKeyDownTrigger(KK_ENTER)) return;
    if (!m_yesSelected) { Close(); return; }
    FPS_SetTimeScale(1.0f);
    if (EngineServiceLocator::ChangeSceneWithFade(SceneManager::SceneID::Title)) {
        m_changingScene = true;
        if (auto* audio = Game::Audio()) { audio->PlaySe(GameSe::MenuConfirm); audio->StopBgm(); }
    }
    else FPS_SetTimeScale(0.0f);
}

void GamePauseControllerBehavior::DrawComponentInspector() {
    ImGui::TextUnformatted(m_open ? "Return confirmation open" : "Backspace: open return confirmation");
}
