//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_effects.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃のエフェクト処理
//---------------------------------------------------
#pragma once
#include "Game/PresBehavior/effect_handle.h"

struct PlayerDualPistolsContext;

class PlayerDualPistolsEffects {
public:
    enum class EffectsType {
        FireLeft,
        FireRight,
        SlashBurst1,
        SlashBurst2,
        SlashBurst3,
    };

private:
    EffectHandle m_muzzleFlashEffect;
    EffectHandle m_slashBurstEffects[3];

    bool m_initialized = false;

public:
    void Initialize(PlayerDualPistolsContext& context);
    void Finalize();
    void PlayEffects(PlayerDualPistolsContext& context, EffectsType effectType);
};
