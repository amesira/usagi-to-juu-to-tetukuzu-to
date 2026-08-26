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

#include "Game/PresBehavior/Camera/camera_control_behavior.h"

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
            { context.settings().aimFOV, 0.0f, 0.0f }, duration);
        CameraEffectTaskHelper::ChangeCameraEffect(context.cameraControlBehavior, CameraEffectTarget::Distance, 
            {context.settings().aimCameraDistance , 0.0f, 0.0f}, duration);
        CameraEffectTaskHelper::ChangeCameraEffect(context.cameraControlBehavior, CameraEffectTarget::CompositionCameraLocalOffset,
            context.settings().aimCameraLocalOffset, duration);
    }

    // 移動リクエストを作成する
    if (context.locomotionController) {
        m_locomotionRequestID = context.locomotionController->AddLocomotionRequest(PlayerLocomotionController::LocomotionRequest{
            .priority = 10,
            .moveDirSourceInfo = { PlayerLocomotionController::DirectionSource::MoveInput },
            .rotateDirSourceInfo = { PlayerLocomotionController::DirectionSource::CameraForward },
            .speedMultiplier = 0.9f,
            .canMove = true,
            .canRotate = true,
            .useGravity = true,
            .applyRotateRightNow = false
            });
    }
}

void PlayerShotgunAim::UpdateAim(PlayerShotgunContext& context, float deltaTime)
{
    // 照準UIの更新とか
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