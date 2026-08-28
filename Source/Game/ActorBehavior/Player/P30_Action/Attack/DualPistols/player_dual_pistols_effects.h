//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_effects.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃のエフェクト処理
//---------------------------------------------------
#pragma once

struct PlayerDualPistolsContext;

class PlayerDualPistolsEffects {
public:
    enum class EffectsType {
        AimEnter,
        AimExit,
        Fire,
    };

    void Initialize(PlayerDualPistolsContext& context);
    void Finalize();
    void PlayEffects(PlayerDualPistolsContext& context, EffectsType effectType);
};
