//===================================================
// File  ：.../Player/Movement/player_move_effects.cpp
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動エフェクト処理を担当するクラス
//===================================================
#include "player_move_effects.h"

#include "Utility/utility_master.h"

#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/rigidbody_component.h"
#include "Engine/Framework/Component/particle_system_component.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"

/// @brief 移動エフェクトを再生する
void PlayerMoveEffects::PlayEffects(PlayerMoveContext context, EffectsType effectType)
{
    switch (effectType) {
    case EffectsType::Jump: {

        break;
        }
    case EffectsType::Land: {

        break;
        }
    case EffectsType::IsUpward: {

        break;
    }
    case EffectsType::IsDownward: {

        break;
    }
        default: break;
    }
}

/// @brief 移動エフェクトの更新処理を行う
void PlayerMoveEffects::UpdateEffects(PlayerMoveContext context, float deltaTime)
{
    if (context.runtimeState.m_isGrounded) {
        SetRunDustParticleActive(context, true);
    }
    else {
        SetRunDustParticleActive(context, false);
    }
}

// ------------

/// @brief 走行時の砂埃パーティクルの有効/無効を設定する
void PlayerMoveEffects::SetRunDustParticleActive(PlayerMoveContext context, bool active)
{
    if (m_isRunDustParticleActive == active) return;

    if (context.references.runDustParticle) {
        auto& desc = context.references.runDustParticle->GetDesc().emissionModule;
        desc.enabled = active;

        m_isRunDustParticleActive = active;
    }
}
