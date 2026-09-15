#include "game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Engine/Device/audio.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Editor/LevelEditor/behavior_detail_view.h"
#include <filesystem>
GameAudioControllerBehavior::~GameAudioControllerBehavior() {
    ReleaseSounds();
    if (Game::Audio() == this) GameControllerLocator::s_audioController = nullptr;
}
void GameAudioControllerBehavior::Setup(const GameAudioSettingsAsset* asset, GameBgm initial) {
    m_settingsAsset = asset; m_requested = initial;
    GameControllerLocator::s_audioController = this;
}
void GameAudioControllerBehavior::Start() {
    GameControllerLocator::s_audioController = this; Reload();
}
void GameAudioControllerBehavior::ReleaseSounds() {
    for (auto& id : m_bgmIds) { UnloadAudio(id); id = -1; }
    for (auto& id : m_seIds) { UnloadAudio(id); id = -1; }
    m_current = GameBgm::None;
}
float GameAudioControllerBehavior::FadeDuration() const { return m_settingsAsset ? m_settingsAsset->GetData().fadeDuration : 0; }
int GameAudioControllerBehavior::CurrentBgmId() const {
    switch (m_current) {
    case GameBgm::Title: return m_bgmIds[0];
    case GameBgm::Battle: return m_bgmIds[1];
    case GameBgm::Battle2: return m_bgmIds[2];
    case GameBgm::Battle3: return m_bgmIds[3];
    case GameBgm::Result: return m_bgmIds[4];
    default: return -1;
    }
}
void GameAudioControllerBehavior::Reload() {
    ReleaseSounds(); m_fade = {};
    m_revision = m_settingsAsset ? m_settingsAsset->GetRevision() : 0;
    if (!m_settingsAsset) { m_status = "No audio settings asset"; return; }
    const auto& s = m_settingsAsset->GetData();
    m_masterVolume = s.masterVolume; m_bgmVolume = s.bgmVolume; m_seVolume = s.seVolume;
    int loaded = 0, failed = 0;
    auto load = [&](const std::string& path) {
        if (path.empty()) return -1;
        const auto wide = std::filesystem::u8path(path).wstring();
        int id = LoadAudio(wide.c_str());
        if (id >= 0) ++loaded;
        else { ++failed; OutputDebugStringW((L"Cannot load PCM WAV: " + wide + L"\n").c_str()); }
        return id;
    };
    m_bgmIds = {load(s.titleBgm), load(s.battleBgm), load(s.battleBgm2), load(s.battleBgm3), load(s.resultBgm)};
    // Keep this list in GameSe order. The size check detects missing entries.
    const int seIds[] = {
        load(s.shotgun),load(s.shotgunAim), load(s.dualPistols), load(s.slashBurst), load(s.slashBurst2),
        load(s.charge), load(s.chargeComplete),
        load(s.playerRun), load(s.playerJump), load(s.playerLand),
        load(s.enemyShot), load(s.enemySlash),
        load(s.playerHit), load(s.enemyHit), load(s.recovery), load(s.getItem),
        load(s.menuMove), load(s.menuConfirm), load(s.menuCancel)
    };
    static_assert(std::size(seIds) == static_cast<size_t>(GameSe::MAX));
    std::copy(std::begin(seIds), std::end(seIds), m_seIds.begin());
    m_status = "Loaded: " + std::to_string(loaded) + " / Failed: " + std::to_string(failed);
    if (!IsAudioInitialized()) m_status += " (audio device unavailable)";
    StartRequestedBgm(); ApplyVolumes();
}
void GameAudioControllerBehavior::StartRequestedBgm() {
    m_current = m_requested;
    const int id = CurrentBgmId();
    m_fade = {};
    if (id >= 0) { SetAudioVolume(id, 0); PlayAudio(id, true); m_fade.Begin(1, FadeDuration()); }
}
void GameAudioControllerBehavior::SetBgm(GameBgm bgm) {
    if (bgm == m_requested) return;
    m_requested = bgm;
    if (m_current == bgm) m_fade.Begin(1, FadeDuration());
    else if (CurrentBgmId() >= 0) m_fade.Begin(0, FadeDuration());
    else StartRequestedBgm();
}
bool GameAudioControllerBehavior::PlaySe(GameSe se) {
    const auto index = static_cast<size_t>(se);
    return GetEnable() && index < m_seIds.size() && PlayAudioOneShot(m_seIds[index], m_seVolume);
}
AudioLoopHandle GameAudioControllerBehavior::StartLoopSe(GameSe se) {
    const auto index = static_cast<size_t>(se);
    if (!GetEnable() || index >= m_seIds.size()) return InvalidAudioLoopHandle;
    return StartAudioLoop(m_seIds[index], m_seVolume);
}
void GameAudioControllerBehavior::StopLoopSe(AudioLoopHandle handle) {
    // Stopping is allowed even when this controller has been disabled.
    StopAudioLoop(handle);
}
void GameAudioControllerBehavior::ApplyVolumes() {
    SetMasterAudioVolume(m_masterVolume);
    SetAudioVolume(CurrentBgmId(), m_bgmVolume * m_fade.value);
    for (int id : m_seIds) SetAudioVolume(id, m_seVolume);
}
void GameAudioControllerBehavior::Update() {
    if (Game::Audio() != this) return;
    if (m_revision != (m_settingsAsset ? m_settingsAsset->GetRevision() : 0)) Reload();
    m_fade.Update(FPS_GetUnscaledDeltaTime());
    if (m_current != m_requested && m_fade.Finished() && m_fade.target == 0) {
        StopAudio(CurrentBgmId()); StartRequestedBgm();
    }
    ApplyVolumes();
}
void GameAudioControllerBehavior::DrawComponentInspector() {
    if (BehaviorDetailView::BeginSection(this, "Game Audio")) {
        ImGui::TextWrapped("%s", m_status.c_str());
        ImGui::TextUnformatted("Settings: asset/Data/game_audio_settings.data.json");
        ImGui::SliderFloat("Master Volume", &m_masterVolume, 0, 1);
        ImGui::SliderFloat("BGM Volume", &m_bgmVolume, 0, 1);
        ImGui::SliderFloat("SE Volume", &m_seVolume, 0, 1);
    }
    BehaviorDetailView::EndSection();
}
