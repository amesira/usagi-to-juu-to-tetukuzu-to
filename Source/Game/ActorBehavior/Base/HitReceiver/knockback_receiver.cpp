//===================================================
// File  ：_/Base/HitReceiver/knockback_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "knockback_receiver.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

void KnockbackReceiver::Initialize(TransformComponent* transform, RigidbodyComponent* rigidbody)
{
    m_transform = transform;
    m_rigidbody = rigidbody;
}

void KnockbackReceiver::Update(float deltaTime)
{
    if (!m_isActive) return;
    if (!m_transform) return;

    DirectX::XMFLOAT3 currentPosition = {};
    if (m_isStartFrame) {
        // 一番最初のフレームは、開始位置を設定する
        m_isStartFrame = false;
        currentPosition = m_startPosition;
    }
    else {
        m_elapsedTime += deltaTime;

        // 位置の更新
        currentPosition = m_transform->GetPosition();
        currentPosition = MiMath::Add(currentPosition, MiMath::Multiply(m_velocity, deltaTime));
        m_velocity.y += m_currentRequest.gravity * deltaTime;
    }

    // 位置の適用
    switch (m_currentRequest.movementMode) {
        case KnockbackMovementMode::SetTransformPosition: {
            m_transform->SetPosition(currentPosition);
            break;
        }
        case KnockbackMovementMode::SetRigidbodyVelocity: {
            if (m_rigidbody && deltaTime > 0.0f) {
                // 速度を直に適用するのではなく、Transformの位置の変化から速度を計算してRigidbodyに設定する
                DirectX::XMFLOAT3 velocity = MiMath::Multiply(MiMath::Subtract(
                    currentPosition,
                    m_transform->GetPosition()
                ), 1.0f / deltaTime);
                m_rigidbody->SetVelocity(velocity);
            }
            break;
        }
        default: break;
    }

    if (m_elapsedTime >= m_currentRequest.duration) {
        m_isActive = false;
        return;
    }
}

bool KnockbackReceiver::StartKnockback(const KnockbackRequest& request)
{
    if (!m_transform) return false;
    if (!request.enabled) return false;
    if (request.duration <= 0.0f) return false;
    if (request.movementMode == KnockbackMovementMode::SetRigidbodyVelocity && !m_rigidbody) return false;

    m_currentRequest = request;
    m_isActive = true;
    m_elapsedTime = 0.0f;
    m_isStartFrame = true;

    // 開始位置と目標位置の計算
    m_startPosition = request.overrideStartPosition ? request.startPosition : m_transform->GetPosition();
    XMFLOAT3 targetPosition = EvaluateTargetPosition(
        m_startPosition, 
        request);
    m_velocity = CalculateInitialVelocity(
        m_startPosition, 
        targetPosition, 
        request.gravity, 
        request.duration);

    return true;
}

void KnockbackReceiver::CancelKnockback()
{
    m_isActive = false;
}

/// @brief ノックバックの初速度を計算する
DirectX::XMFLOAT3 KnockbackReceiver::CalculateInitialVelocity(
    const DirectX::XMFLOAT3& startPosition, 
    const DirectX::XMFLOAT3& targetPosition, 
    float gravity, 
    float duration) const
{
    if (duration <= 0.0f) return { 0.0f, 0.0f, 0.0f };

    DirectX::XMFLOAT3 toTarget = MiMath::Subtract(targetPosition, startPosition);
    DirectX::XMFLOAT3 initialVelocity = MiMath::Multiply(toTarget, 1.0f / duration);
    initialVelocity.y -= 0.5f * gravity * duration;

    return initialVelocity;
}

/// @brief ノックバックの目標位置を評価する
DirectX::XMFLOAT3 KnockbackReceiver::EvaluateTargetPosition(const DirectX::XMFLOAT3& startPosition, const KnockbackRequest& request) const
{
    switch (request.mode) {
        case KnockbackMode::RelativeDistance: {
            return MiMath::Add(startPosition, MiMath::Multiply(request.direction, request.distance));
        }
        case KnockbackMode::TargetPosition: {
            return request.targetPosition;
        }
    default: break;
    }

    return startPosition;
}
