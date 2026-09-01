//===================================================
// File  ：_/DualPistols/player_dual_pistols_slash_burst.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_slash_burst.h"
#include "player_dual_pistols_context.h"
#include "player_dual_pistols_action.h"

#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Player/player_animation_controller.h"

#include "Utility/mi_math.h"

void PlayerDualPistolsSlashBurst::Initialize(PlayerDualPistolsContext& context)
{
    // 初期化処理は必要に応じて実装する
    context.runtimeState.comboStep = 0;
}

void PlayerDualPistolsSlashBurst::Start(PlayerDualPistolsContext& context)
{
    m_isActive = true;
    m_isFinished = false;
    context.runtimeState.comboStep = 0;

    // 攻撃中の移動リクエストを登録
    if (context.locomotionController) {
        m_locomotionRequest = PlayerLocomotionController::LocomotionRequest{
            .priority = 10,
            .canMove = false,
            .canRotate = false,
            .useGravity = false,
            .canJump = false,
        };
        m_locomotionRequestID = context.locomotionController->AddLocomotionRequest(m_locomotionRequest);
    }

    StartNextStep(context);
}

void PlayerDualPistolsSlashBurst::StartNextStep(PlayerDualPistolsContext& context)
{
    m_attackTimer = 0.0f;
    m_fireTimer = context.settings().slashBurstFireInterval; // 最初の発射を即座に行うためにタイマーを初期化

    m_wasReleaseAttackInput = true;
    m_requestNextAttack = false;
    m_hasBursted = false;

    m_isStepMoving = true;
    m_stepMoveTimer = 0.0f;

    // 攻撃のカウントを更新
    context.runtimeState.comboStep++;
    context.runtimeState.comboStep = MiMath::Clamp(context.runtimeState.comboStep, 1, MAX_ATTACK_COUNT);

    // ステップの開始時のみ、移動方向へ身体の向きを変える
    if (context.locomotionController) {
        context.locomotionController->AddForceRotateRequest({
            .priority = 20,
            .directionSourceInfo = {
                .source = PlayerLocomotionController::DirectionSource::MoveInput,
            },
        });

        // 最終攻撃時のみ、重力を有効にする
        m_locomotionRequest.useGravity = (context.runtimeState.comboStep == MAX_ATTACK_COUNT);
    }

    // アニメーションの再生
    switch (context.runtimeState.comboStep) {
    case 1:
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::DualPistolsSlashBurst1);
        break;
    case 2:
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::DualPistolsSlashBurst2);
        break;
    case 3:
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::DualPistolsSlashBurst3);
        break;
    default: break;
    }
}

void PlayerDualPistolsSlashBurst::Update(PlayerDualPistolsContext& context, float deltaTime)
{
    m_attackTimer += deltaTime;

    // 入力解決（次の攻撃入力の予約判定）
    bool releaseAttackInput = context.runtimeState.releaseAttackInput;
    if (releaseAttackInput && !m_wasReleaseAttackInput && CanRequestChainableInput(context)) {
        m_requestNextAttack = true;
    }
    m_wasReleaseAttackInput = releaseAttackInput;

    // 攻撃時の移動ステップ更新
    if (m_isStepMoving) {
        UpdateStepMovement(context, deltaTime);
    }

    // 攻撃の発射処理
    if (m_attackTimer < context.settings().slashBurstFireDuration) {
        m_fireTimer += deltaTime;
        if (m_fireTimer >= context.settings().slashBurstFireInterval) {
            while (m_fireTimer >= context.settings().slashBurstFireInterval) {
                m_fireTimer -= context.settings().slashBurstFireInterval;
                switch (context.runtimeState.comboStep) {
                case 1:
                    FireLeftPistol(context);
                    break;
                case 2:
                    FireRightPistol(context);
                    break;
                case 3:
                    FireVolley(context);
                    break;
                default: break;
                }
            }
        }
    }

    // 攻撃のヒット判定処理
    if (!m_hasBursted && IsBurstFrame(context)) {
        HandleSlashBurstAttack(context, context.runtimeState.comboStep);
    }

    // 次の攻撃への連鎖判定
    if (m_requestNextAttack && IsChainableFrame(context)) {
        // 次の攻撃への連鎖処理
        StartNextStep(context);
        return;
    }

    // 攻撃モーションの終了判定
    if (IsEndMotionFrame(context)) {
        Finish(context);
    }
}

void PlayerDualPistolsSlashBurst::Finish(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = true;

    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }
}

void PlayerDualPistolsSlashBurst::Cancel(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = false;

    m_requestNextAttack = false;
    m_hasBursted = false;

    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }
}

void PlayerDualPistolsSlashBurst::UpdateStepMovement(PlayerDualPistolsContext& context, float deltaTime)
{
    const float stepMoveDuration = (context.runtimeState.comboStep < MAX_ATTACK_COUNT) ? 
        context.settings().stepMoveDuration : context.settings().finalStepMoveDuration;
    const float stepMoveDistance = (context.runtimeState.comboStep < MAX_ATTACK_COUNT) ? 
        context.settings().stepMoveDistance : context.settings().finalStepMoveDistance;

    m_stepMoveTimer += deltaTime;
    if (m_stepMoveTimer >= stepMoveDuration) {
        m_isStepMoving = false;
        return;
    }

    // 移動方向の更新
    if (context.playerTransform && context.locomotionController) {
        XMFLOAT3 moveDirection = context.playerTransform->GetForward();
        moveDirection.y = 0.0f;
        moveDirection = MiMath::Normalize(moveDirection);
        float moveDistancePerFrame = stepMoveDistance / stepMoveDuration * deltaTime;
        context.locomotionController->AddForceMoveRequest({
            .priority = 15,
            .targetPosition = MiMath::Add(context.playerTransform->GetPosition(), MiMath::Multiply(moveDirection, moveDistancePerFrame)),
            });
    }
}

/// @brief 次の連鎖攻撃が入力可能かどうか
bool PlayerDualPistolsSlashBurst::CanRequestChainableInput(PlayerDualPistolsContext& context)
{
    return context.runtimeState.comboStep >= 1 && 
        context.runtimeState.comboStep < MAX_ATTACK_COUNT &&
        IsInputBufferFrame(context);
}

/// @brief 攻撃の実行
bool PlayerDualPistolsSlashBurst::HandleSlashBurstAttack(PlayerDualPistolsContext& context, int step)
{
    if (step < 1 || step > MAX_ATTACK_COUNT) return false;

    m_hasBursted = true;

    // 攻撃ターゲットの検出

    // 攻撃のヒット判定処理
    {
        // ノックバック処理

        // ダメージ処理

        // ヒットストップの再生
    }

    return true;
}

#pragma region 攻撃モーションのフレーム判定
bool PlayerDualPistolsSlashBurst::IsBurstFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().burstTime;
}
bool PlayerDualPistolsSlashBurst::IsInputBufferFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().inputBufferStartTime && m_attackTimer <= context.settings().endTime;
}
bool PlayerDualPistolsSlashBurst::IsChainableFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().chainTime && m_attackTimer <= context.settings().endTime;
}

bool PlayerDualPistolsSlashBurst::IsEndMotionFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().endTime;
}
#pragma endregion

#pragma region 攻撃の発射処理
void PlayerDualPistolsSlashBurst::FireVolley(PlayerDualPistolsContext& context)
{
    FireLeftPistol(context);
    FireRightPistol(context);
}

void PlayerDualPistolsSlashBurst::FireLeftPistol(PlayerDualPistolsContext& context)
{
    PlayerDualPistolsFiring::FireRequest request;
    request.pistolSide = PlayerDualPistolsFiring::PistolSide::Left;

    // SlashBurstでは、発射方向は銃口のボーンの向きに基づいて決定する
    const PlayerMuzzleState& muzzle = context.runtimeState.leftMuzzle;
    if (muzzle.isValid) {
        request.muzzlePosition = muzzle.position;
        request.fireDirection = MiMath::RotateVector(muzzle.rotation, { 0.0f, 1.0f, 0.0f });
        context.firing.Fire(context, request);
    }
}

void PlayerDualPistolsSlashBurst::FireRightPistol(PlayerDualPistolsContext& context)
{
    PlayerDualPistolsFiring::FireRequest request;
    request.pistolSide = PlayerDualPistolsFiring::PistolSide::Right;
    
    const PlayerMuzzleState& muzzle = context.runtimeState.rightMuzzle;
    if (muzzle.isValid) {
        request.muzzlePosition = muzzle.position;
        request.fireDirection = MiMath::RotateVector(muzzle.rotation, { 0.0f, 1.0f, 0.0f });
        context.firing.Fire(context, request);
    }
}
