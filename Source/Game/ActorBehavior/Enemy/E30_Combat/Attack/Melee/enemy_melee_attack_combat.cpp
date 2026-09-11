//===================================================
// File  ：_/E_30_Combat/Attack/Melee/enemy_melee_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_melee_attack_combat.h"

#include "Engine/Component/transform_component.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"

void EnemyMeleeAttackCombat::BeginWindup(EnemyContext& context)
{
    // ピカーンって感じのエフェクトと効果音
}

void EnemyMeleeAttackCombat::EndWindup(EnemyContext& context)
{

}

void EnemyMeleeAttackCombat::BeginAttack(EnemyContext& context)
{
    m_phase = EnemyMeleeAttackPhase::Idle;
    ChangeAttackPhase(EnemyMeleeAttackPhase::Jump);
}

EnemyCombatStatus EnemyMeleeAttackCombat::UpdateAttack(EnemyContext& context, float deltaTime)
{
    m_elapsedTime += deltaTime;

    bool enteredPhase = m_enteredAttackPhase;
    m_enteredAttackPhase = false;

    switch (m_phase) {
    case EnemyMeleeAttackPhase::Jump: {
        if (enteredPhase) {
            BeginJump(context);
        }
        UpdateJump(context, deltaTime);
        if (m_elapsedTime >= settings().jumpDuration) {
            ChangeAttackPhase(EnemyMeleeAttackPhase::Slash);
        }
        return EnemyCombatStatus::Running;
    }
    case EnemyMeleeAttackPhase::Slash: {
        if (enteredPhase) {
            BeginSlash(context);
        }
        UpdateSlash(context, deltaTime);
        return m_elapsedTime >= settings().slashDuration
            ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
    }
    default:
        return EnemyCombatStatus::Failure;
    }
}

void EnemyMeleeAttackCombat::EndAttack(EnemyContext& context)
{
    ClearAttackEffects(context);
    m_phase = EnemyMeleeAttackPhase::Idle;
    m_elapsedTime = 0.0f;
}

/// @brief 攻撃段階を変更する
void EnemyMeleeAttackCombat::ChangeAttackPhase(EnemyMeleeAttackPhase newPhase)
{
    if (m_phase == newPhase) return;
    m_phase = newPhase;
    m_elapsedTime = 0.0f;
    m_enteredAttackPhase = false;
}

void EnemyMeleeAttackCombat::BeginJump(EnemyContext& context)
{
    // LocomotionController
    EnemyLocomotionController::LocomotionRequest request;
    request.priority = 50;

    m_jumpStartPosition = context.transform->GetPosition();
    m_landingPosition = GetAimPosition();

    // ジャンプ時間から初速度を計算する。重力はY軸負方向。
    const float jumpDuration = settings().jumpDuration;
    const float gravity = settings().jumpGravity;
    m_jumpVelocity.x = (m_landingPosition.x - m_jumpStartPosition.x) / jumpDuration;
    m_jumpVelocity.y = (m_landingPosition.y - m_jumpStartPosition.y 
        + 0.5f * gravity * jumpDuration * jumpDuration) / jumpDuration;
    m_jumpVelocity.z = (m_landingPosition.z - m_jumpStartPosition.z) / jumpDuration;
}

void EnemyMeleeAttackCombat::UpdateJump(EnemyContext&, float)
{
    // TODO: 弧を描く移動と着地/衝突を更新する。
}

void EnemyMeleeAttackCombat::BeginSlash(EnemyContext&)
{
    // TODO: スラッシュのアニメーション・攻撃判定を開始する。
}

void EnemyMeleeAttackCombat::UpdateSlash(EnemyContext&, float)
{
    // TODO: 攻撃判定の有効期間を更新する。
}

void EnemyMeleeAttackCombat::ClearAttackEffects(EnemyContext&)
{
    // TODO: 通常終了とキャンセルの両方で移動要求・攻撃判定・演出を解除する。
}
