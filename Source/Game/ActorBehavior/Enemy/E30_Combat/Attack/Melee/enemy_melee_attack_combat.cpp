//===================================================
// File  ：_/E_30_Combat/Attack/Melee/enemy_melee_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_melee_attack_combat.h"

#include "Engine/Component/transform_component.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_locomotion_controller.h"
#include "Game/ActorBehavior/Enemy/enemy_animation_controller.h"

void EnemyMeleeAttackCombat::Initialize(EnemyContext& context)
{
    EnemyAttackCombat::Initialize(context);
    m_effects.Initialize(getContext());
}

void EnemyMeleeAttackCombat::Finalize(EnemyContext& context)
{
    m_slash.Cancel();
    m_effects.Finalize();
}

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
        return UpdateSlash(context, deltaTime);
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

    if (context.locomotionController) {
        context.locomotionController->RemoveRequest(m_locomotionRequestId);
    }
    m_locomotionRequestId = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
    m_enteredAttackPhase = false;

    // アニメーションを元に戻す
    context.animationController->StopCombatAnimation(0.3f);
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

    // ジャンプアニメーション
    context.animationController->PlayCombatAnimation(EnemyAnimationController::Animation::JumpPose, 1.0f);
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
    m_locomotionRequest.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    m_locomotionRequest.rotateDirection.targetPosition = m_landingPosition;
    context.locomotionController->UpdateRequest(m_locomotionRequestId, m_locomotionRequest);
}

void EnemyMeleeAttackCombat::BeginSlash(EnemyContext& context)
{
    // 地上で斬る。ジャンプの重力無効を解除し、向きと水平移動を固定する。
    m_locomotionRequest.canMove = false;
    m_locomotionRequest.canRotate = false;
    m_locomotionRequest.useGravity = true;
    m_locomotionRequest.movementMode = EnemyMovementMode::StopHorizontal;
    if (context.locomotionController
        && !context.locomotionController->UpdateRequest(m_locomotionRequestId, m_locomotionRequest)) {
        m_locomotionRequestId = context.locomotionController->AddRequest(m_locomotionRequest);
    }

    m_slash.Start(getContext());
    m_effects.PlayEffects(EnemyMeleeAttackEffects::EffectsType::Slash);

    // ジャンプアニメーション
    context.animationController->PlayCombatAnimation(EnemyAnimationController::Animation::Slash, 2.0f);
}

EnemyCombatStatus EnemyMeleeAttackCombat::UpdateSlash(EnemyContext& context, float deltaTime)
{
    if (!context.locomotionController
        || m_locomotionRequestId == EnemyLocomotionController::INVALID_REQUEST_HANDLE) {
        return EnemyCombatStatus::Failure;
    }
    m_effects.Update(getContext());

    return m_slash.Update(getContext(), deltaTime);
}

void EnemyMeleeAttackCombat::ClearAttackEffects(EnemyContext& context)
{
    m_slash.Cancel();
    m_effects.StopSlash();
}
