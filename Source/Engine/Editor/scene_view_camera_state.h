// scene_view_camera_state.h
// ・シーンカメラの状態を保持する構造体
#pragma once
#include <DirectXMath.h>

struct SceneViewCameraState {
    DirectX::XMFLOAT3 position = { 0.0f, 13.0f, -13.0f };
    DirectX::XMFLOAT3 atPosition = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 upVector = { 0.0f, 1.0f, 0.0f };
    float fovDegrees = 60.0f;
    float aspect = 16.0f / 9.0f;
    float nearClip = 0.1f;
    float farClip = 300.0f;

    DirectX::XMMATRIX GetViewMatrix() const {
        return DirectX::XMMatrixLookAtLH(
            DirectX::XMLoadFloat3(&position),
            DirectX::XMLoadFloat3(&atPosition),
            DirectX::XMLoadFloat3(&upVector));
    }

    DirectX::XMMATRIX GetProjectionMatrix() const {
        return DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(fovDegrees), aspect, nearClip, farClip);
    }
};
