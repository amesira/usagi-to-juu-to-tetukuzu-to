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

    GameObject* player = context.owner;
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
    m_runDustEffectRate = m_runDustEffect.GetParticleSystem()->Emission().rateOverDistance;
}

void PlayerMoveEffects::Finalize()
{
    m_runDustEffect.Destroy();
    m_runDustEffect.Reset();
    m_isRunDustParticleActive = false;
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

/// @brief 走行時の砂埃パーティクルの有効/無効を設定する
void PlayerMoveEffects::SetRunDustParticleActive(bool active)
{
    if (m_isRunDustParticleActive == active) return;
    if (!m_runDustEffect.IsValid()) return;

    if (active) {
        m_runDustEffect.GetParticleSystem()->Emission().rateOverDistance = m_runDustEffectRate;
        m_runDustEffect.Play();
    }
    else {
        m_runDustEffect.GetParticleSystem()->Emission().rateOverDistance = 0.0f;
    }
    m_isRunDustParticleActive = active;
}

void PlayerMoveEffects::PlayJumpEffect(PlayerMoveContext& context)
{
    XMFLOAT3 spawnPosition = context.transform->GetPosition();
    spawnPosition.y += 0.1f;

    for (int i = 0; i < m_jumpEffects.size(); i++) {
        if (!m_jumpEffects[i].IsPlaying()) {
            m_jumpEffects[i].GetTransform()->SetPosition(spawnPosition);
            m_jumpEffects[i].Play();
            return;
        }
    }

    // 最大数に達している場合は、最初のエフェクトを再利用する
    if (m_jumpEffects.size() >= 8) {
        m_jumpEffects[0].GetTransform()->SetPosition(spawnPosition);
        m_jumpEffects[0].Play();
        return;
    }

    // 新しいエフェクトを作成して再生する
    m_jumpEffects.push_back(RenderEffectFactory::CreateMeshEffect(
        context.scene,
        "asset/MeshEffect/jump_effect.mesh_effect.json",
        {
            .position = spawnPosition,
            .scaling = { 0.6f, 0.6f, 0.6f },
        }));
    m_jumpEffects.back().Play();
}
