//---------------------------------------------------
// File  ：.../Player/Movement/player_move_effects.h
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動エフェクト処理を担当するクラス
//---------------------------------------------------
#pragma once

#include "Game/PresBehavior/attached_effect_handle.h"

class PlayerMoveEffects {
public:
    enum class EffectsType {
        Jump,   // ジャンプ
        Land,   // 着地

        IsUpward,   // 上昇中
        IsDownward, // 下降中
    };

private:
    AttachedEffectHandle m_runDustEffect;
    bool m_isRunDustParticleActive = false; // 走行時の砂埃パーティクルの有効/無効状態を保持する変数

public:
    void Initialize(struct PlayerMoveContext& context);
    void Finalize();

    void PlayEffects(struct PlayerMoveContext& context, EffectsType effectType);
    void UpdateEffects(struct PlayerMoveContext& context, float deltaTime);

private:
    /// @brief 走行時の砂埃パーティクルの有効/無効を設定する
    void SetRunDustParticleActive(bool active);

};
