//===================================================
// File  ：_/Base/HitReceiver/knockback_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "knockback_receiver.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

using namespace HitReceiver;

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
        m_velocity.y += m_currentRequest.movementSource.gravity * deltaTime;
    }

    // === 位置の適用 ===
    switch (m_currentRequest.movementSource.mode) {
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

    // === ノックバック終了判定 ===
    if (m_elapsedTime >= m_currentRequest.duration) {
        m_isActive = false;

        if (m_currentRequest.movementSource.mode == KnockbackMovementMode::SetRigidbodyVelocity && m_rigidbody) {
            m_rigidbody->SetGravityScale(m_rbGravity);
            m_rigidbody->SetFriction(m_rbFriction);
            m_rigidbody->SetVelocity({ 0.0f, 0.0f, 0.0f });
        }
        return;
    }
}

bool KnockbackReceiver::StartKnockback(const KnockbackRequest& request)
{
    if (!m_transform) return false;
    if (!request.enabled) return false;
    if (request.duration <= 0.0f) return false;

    if (m_isLockKnockback) return false;

    KnockbackMovementSource movementSource = request.overrideMovementSource ? request.movementSource : m_defaultMovementSource;
    if (movementSource.mode == KnockbackMovementMode::SetRigidbodyVelocity && !m_rigidbody) return false;

    if (movementSource.mode == KnockbackMovementMode::SetRigidbodyVelocity && m_rigidbody) {
        if (!m_isActive) { // 初回のノックバック開始時にのみ、Rigidbodyのパラメータを保存する
            m_rbMass = m_rigidbody->GetMass();
            m_rbGravity = m_rigidbody->GetGravityScale();
            m_rbFriction = m_rigidbody->GetFriction();
        }
        m_rigidbody->SetGravityScale(0.0f);
        m_rigidbody->SetFriction({ 1.0f, 1.0f, 1.0f });
    }

    // 開始位置と目標位置の計算
    m_startPosition = request.overrideStartPosition ? request.startPosition : m_transform->GetPosition();
    XMFLOAT3 targetPosition = EvaluateTargetPosition(
        m_startPosition, 
        request);
    float gravity = movementSource.gravity;
    m_velocity = CalculateInitialVelocity(
        m_startPosition,
        targetPosition,
        gravity,
        request.duration);

    m_currentRequest = request;
    m_currentRequest.movementSource = movementSource;

    m_isActive = true;
    m_elapsedTime = 0.0f;
    m_isStartFrame = true;
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
