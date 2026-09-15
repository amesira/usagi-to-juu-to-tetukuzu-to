//===================================================
// transform_component.h
// 
// Author：Miu Kitamura
// Date  ：2025/10/27
//===================================================
#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H
#include "Engine/Core/component.h"

#include "Engine/Device/direct3d.h"
using namespace DirectX;

#include <algorithm>
#include "Utility/mi_math.h"

class TransformComponent : public Component {
private:
    // 位置、回転、スケーリングのデータ
    XMFLOAT3   m_position = { 0.0f,0.0f,0.0f };
    XMFLOAT4   m_rotation = { 0.0f,0.0f,0.0f,1.0f };
    XMFLOAT3   m_scaling = { 1.0f,1.0f,1.0f };

    // 前フレームの位置を保存する変数
    XMFLOAT3   m_prevPosition = { 0.0f,0.0f,0.0f };

    XMFLOAT3   m_eulerRawAngle = { 0.0f,0.0f,0.0f };

public:
    // 位置、回転、スケーリングの設定・取得
    void    SetPosition(const XMFLOAT3& position) { m_position = position; }
    XMFLOAT3   GetPosition()const { return m_position; }
    void    SetRotation(const XMFLOAT4& rotation) { m_rotation = rotation; }
    XMFLOAT4   GetRotation()const { return m_rotation; }
    void    SetScaling(const XMFLOAT3& scaling) { m_scaling = scaling; }
    XMFLOAT3   GetScaling()const { return m_scaling; }

    // 前フレームの位置取得・設定
    void    SetPrevPosition(const XMFLOAT3& prevPosition){ m_prevPosition = prevPosition; }
    XMFLOAT3   GetPrevPosition()const { return m_prevPosition; }

    // Forward、Right、Upの取得
    XMFLOAT3   GetForward() const {
        XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f };
        forward = MiMath::RotateVector(m_rotation, forward);
        return forward;
    }
    XMFLOAT3   GetRight() const {
        XMFLOAT3 right = { 1.0f, 0.0f, 0.0f };
        right = MiMath::RotateVector(m_rotation, right);
        return right;
    }
    XMFLOAT3   GetUp() const {
        XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
        up = MiMath::RotateVector(m_rotation, up);
        return up;
    }

    // Euler角での設定・取得（ラジアン角で扱う）
    void    SetEulerAngle(const XMFLOAT3& euler) {
        const float pitch = std::remainder(euler.x, XM_2PI);
        const float yaw = std::remainder(euler.y, XM_2PI);
        const float roll = std::remainder(euler.z, XM_2PI);

        XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

        quaternion = XMQuaternionNormalize(quaternion);
        XMStoreFloat4(&m_rotation, quaternion);
    }
    XMFLOAT3    GetEulerAngle()const {
        XMVECTOR rotation = XMQuaternionNormalize(XMLoadFloat4(&m_rotation));

        XMFLOAT4 q;
        XMStoreFloat4(&q, rotation);

        // Pitch（X）
        const float sinPitch = 2.0f * (q.w * q.x - q.y * q.z);
        const float pitch = std::asin(sinPitch);

        // Yaw（Y）
        const float sinYaw = 2.0f * (q.w * q.y + q.x * q.z);
        const float cosYaw = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
        const float yaw = std::atan2(sinYaw, cosYaw);

        // Roll（Z）
        const float sinRoll = 2.0f * (q.w * q.z + q.x * q.y);
        const float cosRoll = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
        const float roll = std::atan2(sinRoll, cosRoll);

        return { pitch, yaw, roll };
    }
    void    SetEulerRawAngle(const XMFLOAT3& euler) { 
        m_eulerRawAngle = euler;
        SetEulerAngle(euler);
    }
    XMFLOAT3    GetEulerRawAngle()const { return m_eulerRawAngle; }

    // XMVECTORの取得
    XMVECTOR GetRotationVector() const {
        return XMLoadFloat4(&m_rotation);
    }

    XMMATRIX GetWorldMatrix() const {
        XMMATRIX scaling = XMMatrixScaling(m_scaling.x, m_scaling.y, m_scaling.z);
        XMMATRIX rotation = XMMatrixRotationQuaternion(GetRotationVector());
        XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
        return scaling * rotation * translation;
    }

};


#endif