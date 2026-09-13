//===================================================
// File  ：_/DualPistols/player_dual_pistols_slash_burst.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_slash_burst.h"
#include "player_dual_pistols_context.h"
#include "player_dual_pistols_action.h"

#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Component/animation_component.h"
#include <algorithm>

#include "Engine/Component/transform_component.h"
#include "Engine/Component/collider_component.h"
#include "Game/ActorBehavior/Player/player_animation_controller.h"
#include "Game/ActorBehavior/Player/P40_Weapon/player_weapon_controller.h"

#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"

#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"

#include "Utility/mi_math.h"

void PlayerDualPistolsSlashBurst::Initialize(PlayerDualPistolsContext& context)
{
    // 初期化処理は必要に応じて実装する
    context.runtimeState.comboStep = 0;
}

/// @brief 攻撃を開始する
void PlayerDualPistolsSlashBurst::Start(PlayerDualPistolsContext& context)
{
    if (context.weaponController) {
        context.weaponController->SetWeaponMode(
            PlayerWeaponController::WeaponMode::SlashBurst);
    }

    EndHitStop();
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

    if (!StartNextStep(context)) {
        Finish(context);
    }
}

/// @brief 次の攻撃ステップを開始する
bool PlayerDualPistolsSlashBurst::StartNextStep(PlayerDualPistolsContext& context)
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

    // アニメーション・エフェクトの再生
    switch (context.runtimeState.comboStep) {
    case 1:
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::DualPistolsSlashBurst1);
        context.effects.PlayEffects(context, PlayerDualPistolsEffects::EffectsType::SlashBurst1);
        break;
    case 2:
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::DualPistolsSlashBurst2);
        context.effects.PlayEffects(context, PlayerDualPistolsEffects::EffectsType::SlashBurst2);
        break;
    case 3:
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::DualPistolsSlashBurst3);
        context.effects.PlayEffects(context, PlayerDualPistolsEffects::EffectsType::SlashBurst3);
        break;
    default: break;
    }
    return true;
}

/// @brief 攻撃の更新処理
void PlayerDualPistolsSlashBurst::Update(PlayerDualPistolsContext& context, float deltaTime)
{
    // ヒットストップタスク更新
    if (m_hitStopTask.IsRunning()) {
        m_hitStopTask.Update(FPS_GetUnscaledDeltaTime());
        if (m_hitStopTask.IsRunning()) {
            HoldPosition(context);
            return;
        }
        EndHitStop();
    }

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
        if (m_hitStopTask.IsRunning()) return;
    }

    // 次の攻撃への連鎖判定
    if (m_requestNextAttack && IsChainableFrame(context)) {
        // 次の攻撃への連鎖処理
        if (!StartNextStep(context)) {
            Finish(context);
        }
        return;
    }

    // 攻撃モーションの終了判定
    if (IsEndMotionFrame(context)) {
        Finish(context);
    }
}

void PlayerDualPistolsSlashBurst::Finish(PlayerDualPistolsContext& context)
{
    EndHitStop();
    m_isActive = false;
    m_isFinished = true;

    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }
}

void PlayerDualPistolsSlashBurst::Cancel(PlayerDualPistolsContext& context)
{
    EndHitStop();
    m_isActive = false;
    m_isFinished = false;

    m_requestNextAttack = false;
    m_hasBursted = false;

    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }
}

/// @brief 攻撃中の移動ステップを更新する
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

/// @brief 攻撃の実行
bool PlayerDualPistolsSlashBurst::HandleSlashBurstAttack(PlayerDualPistolsContext& context, int step)
{
    if (step < 1 || step > MAX_ATTACK_COUNT) return false;

    m_hasBursted = true;

    XMFLOAT3 attackDir = context.playerTransform->GetForward();
    attackDir = MiMath::HorizontalNormalize(attackDir);

    // 攻撃ターゲットの検出
    std::vector<GameObject*> hitTargets = DetectAttackTarget(context);

    bool isBurstHit = false;
    for (GameObject* target : hitTargets) {
        TransformComponent* targetTransform = target->GetComponent<TransformComponent>();
        HitReceiverBehavior* hitReceiver = target->GetComponent<HitReceiverBehavior>();
        if (targetTransform && hitReceiver) {
            XMFLOAT3 knockbackStartPosition = {};
            XMFLOAT3 knockbackEndPosition = {};
            CalculateKnockbackPosition(
                context,
                targetTransform->GetPosition(),
                attackDir,
                knockbackStartPosition,
                knockbackEndPosition);

            HitReceiver::HitData hitData = {
                .attacker = context.playerTransform->GetOwner(),
                .damage = context.settings().slashBurstDamage,
                .hitDirection = attackDir,
                .knockback = {
                    .enabled = true,
                    .overrideStartPosition = true,
                    .startPosition = knockbackStartPosition,
                    .targetPosition = knockbackEndPosition,
                    .duration = context.settings().slashBurstKnockbackDuration,
                    .mode = HitReceiver::KnockbackMode::TargetPosition,
                    .overrideMovementSource = false,
                },
                .attackType = HitReceiver::AttackType::Slash,
                .hitStop = { .duration = 0.03f, .affectAttacker = true, .affectReceiver = true },
            };
            const auto result = hitReceiver->ReceiveHit(hitData);
            if (result.WasAccepted() && hitData.hitStop.affectAttacker) {
                BeginHitStop(context, hitData.hitStop.duration);
            }
            isBurstHit = true;
        }
    }

    if (isBurstHit) {
        context.effects.PlayEffects(context, PlayerDualPistolsEffects::EffectsType::BurstHit);
    }

    return true;
}

/// @brief 攻撃対象の検出
std::vector<GameObject*> PlayerDualPistolsSlashBurst::DetectAttackTarget(PlayerDualPistolsContext& context)
{
    XMFLOAT3 attackOrigin = context.playerTransform->GetPosition();
    XMFLOAT3 attackOffset = MiMath::RotateVector(context.playerTransform->GetRotation(), context.settings().slashBurstDamageAreaLocalOffset);
    attackOrigin = MiMath::Add(attackOrigin, attackOffset);

    std::vector<ColliderComponent*> outHitTargets = {};
    constexpr CollisionLayerMask layerMask = CollisionLayerToMask(CollisionLayer::Enemy);

    CollisionQuery::OverlapBox(
        context.scene,
        outHitTargets,
        attackOrigin,
        context.settings().slashBurstDamageAreaSize,
        context.playerTransform->GetRotation(),
        layerMask);

    std::vector<GameObject*> hitGameObjects;
    for (ColliderComponent* collider : outHitTargets) {
        GameObject* owner = collider->GetOwner();
        if (std::find(hitGameObjects.begin(), hitGameObjects.end(), owner) == hitGameObjects.end()) {
            hitGameObjects.push_back(owner);
        }
    }

    return hitGameObjects;
}

#pragma region 攻撃のノックバック計算
/// @brief ノックバックの開始位置と終了位置を計算する
void PlayerDualPistolsSlashBurst::CalculateKnockbackPosition(
    const PlayerDualPistolsContext& context,
    const XMFLOAT3& targetPosition,
    const XMFLOAT3& attackDirection,
    XMFLOAT3& outStartPosition,
    XMFLOAT3& outEndPosition) const
{
    outStartPosition = {};
    outEndPosition = {};

    if (!context.playerTransform) return;

    const XMFLOAT3 playerPosition = context.playerTransform->GetPosition();
    const XMFLOAT3 attackDirBack = MiMath::Multiply(attackDirection, -1.0f);

    // 攻撃後方の対象を、攻撃方向側へ移してから吹き飛ばす
    outStartPosition = targetPosition;
    const float distanceBehindPlayer = CalculateDistanceBehindPlayer(
        playerPosition,
        attackDirBack,
        outStartPosition);
    if (distanceBehindPlayer > 0.0f) {
        XMFLOAT3 blowOffset = MiMath::Multiply(attackDirection, distanceBehindPlayer + context.settings().slashBurstKnockbackStartMargin);
        outStartPosition = MiMath::Add(outStartPosition, blowOffset);
    }

    // 終了位置を計算する
    outEndPosition = MiMath::Add(
        outStartPosition,
        MiMath::Multiply(
            attackDirection,
            context.settings().slashBurstKnockbackDistance));

    // 終了位置の攻撃方向成分を、プレイヤーからの最大距離以内に収める。
    // 横方向のずれと高さは維持し、攻撃方向に進み過ぎた分だけを戻す。
    XMFLOAT3 playerToEnd = MiMath::Subtract(outEndPosition, playerPosition);
    playerToEnd.y = 0.0f;
    const float endDistanceAlongAttack = MiMath::Dot(playerToEnd, attackDirection);
    const float maxDistance = context.settings().slashBurstMaxKnockbackDistanceFromPlayer;
    if (endDistanceAlongAttack > maxDistance) {
        outEndPosition = MiMath::Subtract(
            outEndPosition,
            MiMath::Multiply(
                attackDirection,
                endDistanceAlongAttack - maxDistance));
    }

    // 終了位置の高さを、開始位置と同じにする
    outEndPosition.y = outStartPosition.y;
}

/// @brief 対象がプレイヤー後方にいる距離を計算する
float PlayerDualPistolsSlashBurst::CalculateDistanceBehindPlayer(
    const XMFLOAT3& playerPosition,
    const XMFLOAT3& playerBack,
    const XMFLOAT3& targetPosition) const
{
    XMFLOAT3 toTarget = MiMath::Subtract(targetPosition, playerPosition);
    toTarget.y = 0.0f;

    XMFLOAT3 horizontalPlayerBack = playerBack;
    horizontalPlayerBack.y = 0.0f;

    return MiMath::Dot(toTarget, horizontalPlayerBack);
}
#pragma endregion

/// @brief 次の連鎖攻撃が入力可能かどうか
bool PlayerDualPistolsSlashBurst::CanRequestChainableInput(PlayerDualPistolsContext& context)
{
    return context.runtimeState.comboStep >= 1 &&
        context.runtimeState.comboStep < MAX_ATTACK_COUNT &&
        IsInputBufferFrame(context);
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
#pragma endregion

#pragma region ヒットストップ処理
void PlayerDualPistolsSlashBurst::BeginHitStop(PlayerDualPistolsContext& context, float duration)
{
    if (duration <= 0.0f) return;

    // ヒットストップ開始時の状態を保存
    if (!m_hitStopTask.IsRunning()) {
        m_previousCanMove = m_locomotionRequest.canMove;
        m_previousUseGravity = m_locomotionRequest.useGravity;
        m_hitStopAnimation = context.animationController->GetAnimationComponent();
        if (m_hitStopAnimation) {
            m_hitStopAnimation->SetPaused(true);
        }
    }

    m_hitStopTask.RequestHold((std::max)(duration, m_hitStopTask.GetRemainingTime()));
    HoldPosition(context);
}

void PlayerDualPistolsSlashBurst::HoldPosition(PlayerDualPistolsContext& context)
{
    m_locomotionRequest.canMove = false;
    m_locomotionRequest.useGravity = false;
    m_locomotionRequest.pauseMovement = true;
}

void PlayerDualPistolsSlashBurst::EndHitStop()
{
    if (m_hitStopAnimation) {
        m_hitStopAnimation->SetPaused(false);
        m_hitStopAnimation = nullptr;
    }

    if (m_hitStopTask.IsRunning() || m_hitStopTask.m_duration > 0.0f) {
        m_locomotionRequest.canMove = m_previousCanMove;
        m_locomotionRequest.useGravity = m_previousUseGravity;
    }
    m_locomotionRequest.pauseMovement = false;
    m_hitStopTask.Reset();
    m_hitStopTask.m_duration = 0.0f;
}
#pragma endregion
