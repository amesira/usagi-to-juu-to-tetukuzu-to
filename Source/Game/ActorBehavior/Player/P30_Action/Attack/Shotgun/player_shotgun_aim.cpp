//===================================================
// File  ：_/Shotgun/player_shotgun_aim.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの照準を制御するクラス
//===================================================
#include "player_shotgun_aim.h"
#include "player_shotgun_context.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/camera_component.h"
#include "Game/PresBehavior/Camera/camera_control_behavior.h"

#include "Utility/mi_math.h"

namespace {
    using CameraEffectTarget = CameraEffect::EffectTaskTarget;
}

void PlayerShotgunAim::EnterAim(PlayerShotgunContext& context)
{
    if (m_isAiming) return;
    m_isAiming = true;

    // エイムモードのためのカメラエフェクトを設定する
    if (context.cameraControlBehavior) {
        float duration = context.settings().aimTransitionTime;
        CameraEffectTaskHelper::ChangeCameraEffect(context.cameraControlBehavior, CameraEffectTarget::FOV, 
            context.settings().aimFOV, duration);
        CameraEffectTaskHelper::ChangeCameraEffect(context.cameraControlBehavior, CameraEffectTarget::Distance, 
            context.settings().aimCameraDistance, duration);
        CameraEffectTaskHelper::ChangeCameraEffect(context.cameraControlBehavior, CameraEffectTarget::CompositionCameraLocalOffset,
            context.settings().aimCameraLocalOffset, duration);
    }

    // 移動リクエストを作成する
    if (context.locomotionController) {
        m_locomotionRequest = PlayerLocomotionController::LocomotionRequest{
            .priority = 10,
            .moveDirSourceInfo = { PlayerLocomotionController::DirectionSource::MoveInput },
            .rotateDirSourceInfo = { PlayerLocomotionController::DirectionSource::CameraForward },
            .speedMultiplier = 0.9f,
            .canMove = true,
            .canRotate = true,
            .useGravity = true,
            .applyRotateRightNow = false
        };
        m_locomotionRequestID = context.locomotionController->AddLocomotionRequest(m_locomotionRequest);
    }
}

void PlayerShotgunAim::UpdateAim(PlayerShotgunContext& context, float deltaTime)
{
    // 照準UIの更新とか

    // AimResultの更新
    m_aimResult.cameraRayOrigin = context.cameraTransform->GetPosition();
    m_aimResult.cameraRayDirection = context.cameraTransform->GetForward();
    m_aimResult.muzzlePosition = context.references.muzzleTransform->GetPosition();
    m_aimResult.targetPosition = MiMath::Add(context.cameraTransform->GetPosition(), MiMath::Multiply(context.cameraTransform->GetForward(), 1000.0f));
    m_aimResult.fireDirection = MiMath::Normalize(MiMath::Subtract(m_aimResult.targetPosition, m_aimResult.muzzlePosition));
}

void PlayerShotgunAim::ExitAim(PlayerShotgunContext& context)
{
    if (!m_isAiming) return;
    m_isAiming = false;

    // エイムモードのカメラエフェクトをリセットする
    if (context.cameraControlBehavior) {
        float duration = context.settings().aimTransitionTime;
        CameraEffectTaskHelper::ResetCameraEffect(context.cameraControlBehavior, CameraEffectTarget::FOV, duration);
        CameraEffectTaskHelper::ResetCameraEffect(context.cameraControlBehavior, CameraEffectTarget::Distance, duration);
        CameraEffectTaskHelper::ResetCameraEffect(context.cameraControlBehavior, CameraEffectTarget::CompositionCameraLocalOffset, duration);
    }

    // 移動リクエストをリセットする
    if (context.locomotionController && m_locomotionRequestID != -1) {
        context.locomotionController->RemoveLocomotionRequestByIndex(m_locomotionRequestID);
        m_locomotionRequestID = -1;
    }
}