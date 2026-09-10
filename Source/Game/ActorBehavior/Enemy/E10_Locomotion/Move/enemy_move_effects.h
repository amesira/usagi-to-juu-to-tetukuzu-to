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
private:
    AttachedEffectHandle m_runDustEffect;
    bool m_isRunDustParticleActive = false;
    float m_runDustEffectRate = 0.0f;

public:
    void Initialize(struct PlayerMoveContext& context);
    void Finalize();

    void UpdateEffects(struct PlayerMoveContext& context, float deltaTime);

private:
    void SetRunDustParticleActive(bool active);
    void PlayJumpEffect(struct PlayerMoveContext& context);

};
