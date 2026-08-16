//---------------------------------------------------
// File  ：_/PresBehavior/Camera/camera_context.h
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・カメラの状態や設定を管理するコンテキストクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

// 構成のためのサブクラス
#include "camera_settings_asset.h"
#include "camera_effect.h"

struct CameraRuntimeState
{
    // ユーザーによるカメラ回転入力を受け付けるか
    bool isInputEnabled = true;

    DirectX::XMFLOAT3 currentCameraPivotPosition = {};
    DirectX::XMFLOAT3 currentCameraEyePosition = {};
    DirectX::XMFLOAT3 cameraForward = {};
    DirectX::XMFLOAT3 cameraRight = {};

    // 現在のカメラ構図
    DirectX::XMFLOAT3 lookAtOffset = {};
    DirectX::XMFLOAT3 lookAtLocalOffset = {};
    float followDistance = 10.0f;
    float fov = 80.0f;

    // 現在角度と目標角度
    float pitch = 0.0f;
    float yaw = 0.0f;
    float targetPitch = 0.0f;
    float targetYaw = 0.0f;

    // SmoothDampの内部状態
    float pitchVelocity = 0.0f;
    float yawVelocity = 0.0f;

    DirectX::XMFLOAT3 pivotPositionVelocity = {};
    DirectX::XMFLOAT3 cameraPositionVelocity = {};

    void Initialize(const CameraSettings::Data& settings)
    {
        isInputEnabled = true;

        lookAtOffset = settings.lookAtOffset;
        lookAtLocalOffset = settings.lookAtLocalOffset;
        followDistance = settings.followDistance;
        fov = settings.fov;

        pitch = settings.initialPitch;
        yaw = settings.initialYaw;
        targetPitch = pitch;
        targetYaw = yaw;

        ResetSmoothDampVelocity();
    }

    void ResetSmoothDampVelocity()
    {
        pitchVelocity = 0.0f;
        yawVelocity = 0.0f;
        pivotPositionVelocity = {};
        cameraPositionVelocity = {};
    }
};

struct CameraReferences
{
    // 追従ターゲットのTransformComponent
    class TransformComponent* targetTransform = nullptr;
    // 注視を補助するフォーカスターゲット
    class TransformComponent* focusTarget = nullptr;
};

class CameraContext {
public:
    class CameraControlBehavior* owner = nullptr;
    class TransformComponent* transform = nullptr;
    class CameraComponent* camera = nullptr;

    CameraRuntimeState runtimeState;
    CameraReferences references;
    const CameraSettingsAsset* settingsAsset = nullptr;
    const auto& settings() const {
        return settingsAsset->GetData();
    }

    CameraEffect cameraEffect;
};
