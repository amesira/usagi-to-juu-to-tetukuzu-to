//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_effects.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンのエフェクトを制御するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/PresBehavior/attached_effect_handle.h"
#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Engine/Core/GamePlay/tween_task.h"

struct PlayerShotgunContext;

class PlayerShotgunEffects {
public:
    enum class EffectsType {
        AimEnter,     // エイムモードに入る
        AimExit,      // エイムモードから出る
        StartCharge,  // チャージ開始
        ResetCharge,  // チャージリセット
        ChargeComplete, // チャージ完了
        Fire,         // 発射

        ChargeCompleteFire, // チャージ完了状態での発射
        ChargeCompleteFireFinish, // チャージ完了状態での発射後のエフェクト終了
    };

private:
    int m_settingsRevisionCounter = -1;

    AttachedEffectHandle m_chargeEffect;
    AttachedEffectHandle m_chargeCompleteEffect;
    AttachedEffectHandle m_muzzleFlashEffect;

    AudioLoopHandle m_chargeLoopSe;

    WaitAndCallbackTask m_chargeCompleteEffectTask;
    bool m_playChargeCompleteFireFinishEffect = false;

    bool m_initialized = false;

public:
    void Initialize(PlayerShotgunContext& context);
    void Update(PlayerShotgunContext& context);
    void Finalize();
    void PlayEffects(PlayerShotgunContext& context, EffectsType effectType);

};
