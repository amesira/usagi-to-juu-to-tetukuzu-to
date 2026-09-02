//===================================================
// File  ：.../Player/Movement/player_move_effects.cpp
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動エフェクト処理を担当するクラス
//===================================================
#include "player_move_effects.h"

#include "Utility/utility_master.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/particle_system_component.h"
#include "Engine/Core/game_object.h"

#include "Game/Factory/render_effect_factory.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_behavior.h"

/// @brief 移動エフェクトを再生する
void PlayerMoveEffects::Initialize(PlayerMoveContext& context)
{
    if (!context.owner || !context.transform || !context.settingsAsset) return;

    GameObject* player = context.owner->GetOwner();
    if (!player || !player->GetScene()) return;

    const auto& settings = context.settings().runDustEffect;
    m_runDustEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        player->GetScene(),
        settings.particleAssetPath,
        EffectAttachmentDesc{
            .target = context.transform,
            .localTransform = {
                .position = settings.positionOffset,
            },
        });
    m_isRunDustParticleActive = false;
}

void PlayerMoveEffects::Finalize()
{
    m_runDustEffect.Destroy();
    m_runDustEffect.Reset();
    m_isRunDustParticleActive = false;
}

void PlayerMoveEffects::PlayEffects(PlayerMoveContext& context, EffectsType effectType)
{
    switch (effectType) {
    case EffectsType::Jump: {
        // TODO: リファレンス内のジャンプエフェクトを複製し、再生する
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
void PlayerMoveEffects::UpdateEffects(PlayerMoveContext& context, float deltaTime)
{
    if (context.runtimeState.m_isGrounded) {
        SetRunDustParticleActive(true);
    }
    else {
        SetRunDustParticleActive(false);
    }
}

// ------------

/// @brief 走行時の砂埃パーティクルの有効/無効を設定する
void PlayerMoveEffects::SetRunDustParticleActive(bool active)
{
    if (m_isRunDustParticleActive == active) return;

    if (!m_runDustEffect.IsValid()) return;

    if (active) {
        m_runDustEffect.Play();
    }
    else {
        m_runDustEffect.Stop();
    }
    m_isRunDustParticleActive = active;
}
