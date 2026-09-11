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

    context.locomotionController->RemoveRequest(m_locomotionRequestId);
}

/// @brief 攻撃段階を変更する
void EnemyMeleeAttackCombat::ChangeAttackPhase(EnemyMeleeAttackPhase newPhase)
{
    if (m_phase == newPhase) return;
    m_phase = newPhase;
    m_elapsedTime = 0.0f;
    m_enteredAttackPhase = true;
}

void EnemyMeleeAttackCombat::BeginJump(EnemyContext& context)
{
    // 移動要求：ジャンプ中はForceMoveにより移動するので、LocomotionRequestでは移動を無効化
    m_locomotionRequest.priority = 50;
    m_locomotionRequest.canMove = false;
    m_locomotionRequest.canRotate = true;
    m_locomotionRequest.useGravity = false;
    m_locomotionRequest.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    m_locomotionRequest.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    m_locomotionRequestId = context.locomotionController->AddRequest(m_locomotionRequest);

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

void EnemyMeleeAttackCombat::UpdateJump(EnemyContext& context, float deltaTime)
{
    XMFLOAT3 currentPosition = context.transform->GetPosition();
    currentPosition = MiMath::Add(currentPosition, MiMath::Multiply(m_jumpVelocity, deltaTime));
    m_jumpVelocity.y -= settings().jumpGravity * deltaTime;

    // ForceMove要求で移動
    context.locomotionController->AddForceMoveRequest({
        .priority = 50,
        .targetPosition = currentPosition
        });

    // 速度方向に回転更新
    m_locomotionRequest.rotateDirection.source = EnemyLocomotionController::DirectionSource::FixedDirection;
    m_locomotionRequest.rotateDirection.fixedDirection = m_jumpVelocity;
    context.locomotionController->UpdateRequest(m_locomotionRequestId, m_locomotionRequest);
}

void EnemyMeleeAttackCombat::BeginSlash(EnemyContext& context)
{
    // TODO: スラッシュのアニメーション・攻撃判定を開始する。

    // アニメーション再生

    // エフェクト再生

}

void EnemyMeleeAttackCombat::UpdateSlash(EnemyContext& context, float deltaTime)
{
    // TODO: 攻撃判定の有効期間を更新する。

    // 攻撃判定発生

}

void EnemyMeleeAttackCombat::ClearAttackEffects(EnemyContext& context)
{
    // TODO: 通常終了とキャンセルの両方で移動要求・攻撃判定・演出を解除する

}
