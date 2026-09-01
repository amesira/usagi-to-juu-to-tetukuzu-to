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
    };

private:
    int m_settingsRevisionCounter = -1;

    AttachedEffectHandle m_chargeEffect;
    AttachedEffectHandle m_chargeCompleteEffect;
    AttachedEffectHandle m_muzzleFlashEffect;

    bool m_initialized = false;

public:
    void Initialize(PlayerShotgunContext& context);
    void Update(PlayerShotgunContext& context);
    void Finalize();
    void PlayEffects(PlayerShotgunContext& context, EffectsType effectType);

};
