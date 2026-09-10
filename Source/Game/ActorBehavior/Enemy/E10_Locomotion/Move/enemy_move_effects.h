//---------------------------------------------------
// File  ：_/E10_Locomotion/Move/enemy_move_effects.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・EnemyMoveBehaviorの移動エフェクト処理を担当するクラス
//---------------------------------------------------
#pragma once
#include <vector>
#include "Game/PresBehavior/attached_effect_handle.h"

class EnemyMoveEffects {
private:
    AttachedEffectHandle m_runDustEffect;
    bool m_isRunDustParticleActive = false;
    float m_runDustEffectRate = 0.0f;

public:
    void Initialize(struct EnemyMoveContext& context);
    void Finalize();

    void UpdateEffects(struct EnemyMoveContext& context, float deltaTime);

private:
    void SetRunDustParticleActive(bool active);

};
