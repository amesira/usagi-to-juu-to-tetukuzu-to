//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_effects.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンのエフェクトを制御するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

struct PlayerShotgunContext;

class PlayerShotgunEffects {
public:
    enum class EffectsType {
        AimEnter,   // エイムモードに入る
        StartCharge,  // チャージ開始
        ResetCharge,  // チャージリセット
        ChargeComplete, // チャージ完了
    };

private:

public:
    void PlayEffects(PlayerShotgunContext& context, EffectsType effectType);

};