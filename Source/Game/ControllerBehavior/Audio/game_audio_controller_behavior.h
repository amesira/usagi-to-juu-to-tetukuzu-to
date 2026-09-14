#pragma once
#include "Engine/Component/behavior_component.h"
#include "game_audio_settings_asset.h"
#include "audio_fade.h"
#include <array>

enum class GameBgm { 
    None, 
    Title,
    Battle,
    Result,
};

enum class GameSe { 
    Shotgun, 
    DualPistols, 
    SlashBurst,

    PlayerRun,
    PlayerJump,
    PlayerLand,

    EnemyShot,
    EnemySlash,

    PlayerHit,
    EnemyHit,

    Recovery,
    GetItem,

    MenuMove, 
    MenuConfirm, 
    MenuCancel, 

    MAX,
};

class GameAudioControllerBehavior : public BehaviorComponent {
private:
    const GameAudioSettingsAsset* m_settingsAsset = nullptr;

    std::array<int, 3> m_bgmIds{-1, -1, -1};
    std::array<int, static_cast<size_t>(GameSe::MAX)> m_seIds;

    GameBgm m_requested = GameBgm::None, m_current = GameBgm::None;
    AudioFade m_fade;
    float m_masterVolume = 1, m_bgmVolume = .5f, m_seVolume = 1;

    std::uint64_t m_revision = 0;
    std::string m_status;

public:
    GameAudioControllerBehavior() { m_seIds.fill(-1); }
    ~GameAudioControllerBehavior() override;
    void Setup(const GameAudioSettingsAsset* asset, GameBgm initialBgm);
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    void SetBgm(GameBgm bgm);
    void StopBgm() { SetBgm(GameBgm::None); }
    bool PlaySe(GameSe se);

private:
    void Reload();
    void ReleaseSounds();
    void StartRequestedBgm();
    int CurrentBgmId() const;
    float FadeDuration() const;
    void ApplyVolumes();

};
