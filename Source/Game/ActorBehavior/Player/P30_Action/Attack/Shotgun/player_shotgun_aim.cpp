//===================================================
// File  ：_/Shotgun/player_shotgun_aim.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの照準を制御するクラス
//===================================================
#include "player_shotgun_aim.h"
#include "player_shotgun_context.h"

#include "Game/ActorBehavior/Player/player_animation_controller.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/camera_component.h"
#include "Engine/Graphics/model_animation_utility.h"
#include "Game/PresBehavior/Camera/camera_control_behavior.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/custom_post_effect_controller.h"

#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"

#include "Utility/mi_math.h"

namespace {
    using CameraEffectTarget = CameraEffect::EffectTaskTarget;

    constexpr CollisionLayerMask AIM_LAYER_MASK =
        COLLISION_LAYER_MASK_ALL &
        ~CollisionLayerToMask(CollisionLayer::Player) &
        ~CollisionLayerToMask(CollisionLayer::Bullet);
}

void PlayerShotgunAim::EnterAim(PlayerShotgunContext& context)
{
    if (m_isAiming) return;
    m_isAiming = true;

    // エイムモードのためのカメラを設定する
    SetAimingCameraSetting(context, true);

    // 移動リクエストを作成する
    if (context.locomotionController) {
        m_locomotionRequest = PlayerLocomotionController::LocomotionRequest{
            .priority = 10,
            .moveDirSourceInfo = { PlayerLocomotionController::DirectionSource::MoveInput },
            .rotateDirSourceInfo = { PlayerLocomotionController::DirectionSource::CameraForward },
            .speedMultiplier = context.settings().aimMoveSpeedMultiplier,
            .jumpPowerMultiplier = context.settings().aimJumpPowerMultiplier,
            .canMove = true,
            .canRotate = true,
            .useGravity = true,
            .canJump = true,
            .applyRotateRightNow = true,
        };
        m_locomotionRequestID = context.locomotionController->AddLocomotionRequest(m_locomotionRequest);
    }

    context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::AimEnter);
}

void PlayerShotgunAim::UpdateAim(PlayerShotgunContext& context, float deltaTime)
{
    // TODO: 照準UIの更新とか

    if (!context.cameraTransform || !context.cameraComponent) {
        m_aimResult = {};
        return;
    }

    // カメラ中央からRaycastし、命中点をマズルから狙う。
    m_aimResult.cameraRayOrigin = context.cameraTransform->GetPosition();
    m_aimResult.cameraRayDirection = MiMath::Normalize(context.cameraComponent->GetForward());

    ModelAnimationUtility::BoneTransform gunTransform;
    const bool foundGunBone =
        context.playerModel &&
        context.playerTransform &&
        ModelAnimationUtility::GetBoneWorldTransform(
            *context.playerModel,
            *context.playerTransform,
            context.references.gunLBoneIndex,
            gunTransform);

    if (foundGunBone) {
        m_aimResult.muzzlePosition = gunTransform.position;
    }
    else {
        m_aimResult = {};
        return;
    }

    RaycastHit hit;
    m_aimResult.hasTargetHit = CollisionQuery::Raycast(
        context.scene,
        hit,
        m_aimResult.cameraRayOrigin,
        m_aimResult.cameraRayDirection,
        context.settings().aimMaxDistance,
        AIM_LAYER_MASK);

    if (m_aimResult.hasTargetHit) {
        m_aimResult.targetPosition = hit.hitPoint;
    }
    else {
        m_aimResult.targetPosition = MiMath::Add(
            m_aimResult.cameraRayOrigin,
            MiMath::Multiply(m_aimResult.cameraRayDirection, context.settings().aimMaxDistance));
    }

    m_aimResult.fireDirection = MiMath::Normalize(
        MiMath::Subtract(m_aimResult.targetPosition, m_aimResult.muzzlePosition));
}

void PlayerShotgunAim::ExitAim(PlayerShotgunContext& context)
{
    if (!m_isAiming) return;
    m_isAiming = false;

    // エイムモードのカメラ設定をリセットする
    SetAimingCameraSetting(context, false);

    // 移動リクエストをリセットする
    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }

    context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::AimExit);
}

/// @brief エイムモードのためのカメラを設定する
void PlayerShotgunAim::SetAimingCameraSetting(PlayerShotgunContext& context, bool enable)
{
    if (!context.cameraControlBehavior) return;
    if (enable) {
        context.cameraControlBehavior->SetSettingsAsset(context.shotgunCameraSettingsAsset);
    }
    else {
        context.cameraControlBehavior->SetSettingsAsset(context.defaultCameraSettingsAsset);
    }
}

/// @brief エイム中のアニメーションを更新する
void PlayerShotgunAim::UpdateAimingAnimation(PlayerShotgunContext& context, float deltaTime)
{
    if (!context.animationController || !context.playerRuntimeState) return;
    if (!context.playerRuntimeState->m_isGrounded) return;

    const auto& settings = context.settings();
    const float directionY = m_aimResult.fireDirection.y;

    // エイムモード中の移動方向のBlendTreeパラメータを滑らかに更新する
    const DirectX::XMFLOAT2& targetMoveParameter =
        context.playerRuntimeState->localMoveParameter;
    m_moveBlendParameter.x = MiMath::SmoothDamp(
        m_moveBlendParameter.x,
        targetMoveParameter.x,
        m_moveBlendParameterVelocity.x,
        0.1f,
        deltaTime);
    m_moveBlendParameter.y = MiMath::SmoothDamp(
        m_moveBlendParameter.y,
        targetMoveParameter.y,
        m_moveBlendParameterVelocity.y,
        0.1f,
        deltaTime);

    // 上下方向のエイム姿勢をBlendTreeで制御するためのパラメータを計算する
    float targetAimBlendParameter = 0.0f;

    if (directionY < settings.aimBlendDownStartDirectionY) {
        const float downRange =
            settings.aimBlendDownStartDirectionY -
            settings.aimBlendDownFullDirectionY;

        if (downRange > 0.0f) {
            targetAimBlendParameter = -MiMath::Clamp(
                (settings.aimBlendDownStartDirectionY - directionY) /
                downRange,
                0.0f,
                1.0f);
        }
    }
    else if (directionY > settings.aimBlendUpStartDirectionY) {
        const float upRange =
            settings.aimBlendUpFullDirectionY -
            settings.aimBlendUpStartDirectionY;

        if (upRange > 0.0f) {
            targetAimBlendParameter = MiMath::Clamp(
                (directionY - settings.aimBlendUpStartDirectionY) /
                upRange,
                0.0f,
                1.0f);
        }
    }
    m_aimBlendParameter = MiMath::SmoothDamp(
        m_aimBlendParameter,
        targetAimBlendParameter,
        m_aimBlendParameterVelocity,
        0.1f,
        deltaTime);

    // 移動方向の基礎姿勢へ、上半身の上下エイム姿勢を重ねる。
    context.animationController->PlayBlendTree2D(
        PlayerAnimationController::Animation::Aiming,
        m_moveBlendParameter);
    context.animationController->PlayLayerBlendTree1D(
        PlayerAnimationController::AnimationLayer::ShotgunAimVertical,
        m_aimBlendParameter);
}
