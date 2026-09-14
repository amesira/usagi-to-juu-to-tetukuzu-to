//===================================================
// File  ：_/E10_Locomotion/Move/enemy_move_effects.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_move_effects.h"

#include "Utility/utility_master.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/particle_system_component.h"
#include "Engine/Core/game_object.h"

#include "Game/Factory/render_effect_factory.h"

#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move_intent.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move.h"

/// @brief 移動エフェクトを再生する
void EnemyMoveEffects::Initialize(EnemyMoveContext& context)
{
    m_stopped = false;
    if (!context.owner || !context.transform || !context.settingsAsset) return;

    GameObject* enemy = context.owner;
    if (!enemy || !enemy->GetScene()) return;

    const auto& settings = context.settings().runEffect;
    m_runDustEffect = RenderEffectFactory::CreateAttachedParticleEffect(
        enemy->GetScene(),
        settings.particleAssetPath,
        EffectAttachmentDesc{
            .target = context.transform,
            .localTransform = {
                .position = settings.positionOffset,
            },
        });
    m_isRunDustParticleActive = false;
    if (auto* particles = m_runDustEffect.GetParticleSystem()) {
        m_runDustEffectRate = particles->Emission().rateOverDistance;
    }
}

void EnemyMoveEffects::Stop()
{
    m_stopped = true;
    m_runDustEffect.Stop();
    m_isRunDustParticleActive = false;
}

void EnemyMoveEffects::Finalize()
{
    Stop();
    m_runDustEffect.Destroy();
    m_runDustEffect.Reset();
    m_isRunDustParticleActive = false;
}

/// @brief 移動エフェクトの更新処理を行う
void EnemyMoveEffects::UpdateEffects(EnemyMoveContext& context, float deltaTime)
{
    if (m_stopped) return;
    if (context.runtimeState.isGrounded) {
        SetRunDustParticleActive(true);
    }
    else {
        SetRunDustParticleActive(false);
    }
}

/// @brief 走行時の砂埃パーティクルの有効/無効を設定する
void EnemyMoveEffects::SetRunDustParticleActive(bool active)
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