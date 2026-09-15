#pragma once
#include "Engine/Component/behavior_component.h"
#include "game_audio_settings_asset.h"
#include "audio_fade.h"
#include "Engine/Device/audio.h"
#include <array>

enum class GameBgm { 
    None, 
    Title,
    Battle,
    Battle2,
    Battle3,
    Result,
};

enum class GameSe { 
    Shotgun, 
    ShotgunAim,
    DualPistols, 
    SlashBurst,
    SlashBurst2,

    Charge,
    ChargeComplete,

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

    std::array<int, 5> m_bgmIds{-1, -1, -1, -1, -1};
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
    // Start once on a state change; retain the handle to stop this instance.
    // Returns InvalidAudioLoopHandle when the sound cannot be started.
    AudioLoopHandle StartLoopSe(GameSe se);
    void StopLoopSe(AudioLoopHandle handle);

private:
    void Reload();
    void ReleaseSounds();
    void StartRequestedBgm();
    int CurrentBgmId() const;
    float FadeDuration() const;
    void ApplyVolumes();

};
