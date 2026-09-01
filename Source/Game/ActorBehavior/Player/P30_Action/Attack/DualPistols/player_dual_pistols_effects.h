//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_effects.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃のエフェクト処理
//---------------------------------------------------
#pragma once
#include <vector>
#include "Game/PresBehavior/effect_handle.h"
#include "Game/PresBehavior/attached_effect_handle.h"

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
    int m_settingsRevisionCounter = -1;

    std::vector<EffectHandle> m_muzzleFlashEffects;
    AttachedEffectHandle m_slashBurstEffect1;
    AttachedEffectHandle m_slashBurstEffect2;
    AttachedEffectHandle m_slashBurstEffect3[2]; // 0: 左ピストル, 1: 右ピストル

    bool m_initialized = false;

public:
    void Initialize(PlayerDualPistolsContext& context);
    void Update(PlayerDualPistolsContext& context);
    void Finalize();
    void PlayEffects(PlayerDualPistolsContext& context, EffectsType effectType);

private:
    void PlayMuzzleFlashEffect(PlayerDualPistolsContext& context, int pistolIndex);

};
