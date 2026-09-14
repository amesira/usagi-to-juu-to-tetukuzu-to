//---------------------------------------------------
// File  ：.../Player/Movement/player_move_effects.h
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動エフェクト処理を担当するクラス
//---------------------------------------------------
#pragma once
#include <vector>
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
    bool m_isRunEffectActive = false;

    AttachedEffectHandle m_runDustEffect;
    float m_runDustEffectRate = 0.0f;
    int m_runLoopSeHandle = -1;

    std::vector<EffectHandle> m_jumpEffects;

public:
    void Initialize(struct PlayerMoveContext& context);
    void Finalize();

    void PlayEffects(struct PlayerMoveContext& context, EffectsType effectType);
    void UpdateEffects(struct PlayerMoveContext& context, float deltaTime);

private:
    void SetRunEffectActive(bool active);
    void PlayJumpEffect(struct PlayerMoveContext& context);

};
