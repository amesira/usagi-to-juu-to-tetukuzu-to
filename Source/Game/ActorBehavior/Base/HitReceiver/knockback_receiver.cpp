//===================================================
// File  ：_/Base/HitReceiver/knockback_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "knockback_receiver.h"
#include <cmath>

#include "Engine/Component/transform_component.h"
#include "Game/ControllerBehavior/StageBounds/stage_bounds_controller_behavior.h"
#include "Engine/Component/rigidbody_component.h"

using namespace HitReceiver;

void KnockbackReceiver::Initialize(TransformComponent* transform, RigidbodyComponent* rigidbody)
{
    m_transform = transform;
    m_rigidbody = rigidbody;
}

void KnockbackReceiver::Update(float deltaTime, float unscaledDeltaTime)
{
    if (!std::isfinite(deltaTime) || deltaTime < 0.0f) return;
    if (!std::isfinite(unscaledDeltaTime) || unscaledDeltaTime < 0.0f) return;

    switch (m_state) {
    case State::WaitingDelay:
        UpdateDelay(unscaledDeltaTime);
        break;
    case State::Moving:
        UpdateMovement(deltaTime);
        break;
    case State::Inactive:
    default:
        break;
    }
}

void KnockbackReceiver::UpdateDelay(float unscaledDeltaTime)
{
    m_delayElapsedTime += unscaledDeltaTime;
    if (m_delayElapsedTime >= m_currentRequest.delay) BeginMovement();
}

void KnockbackReceiver::UpdateMovement(float deltaTime)
{
    if (!m_transform) {
        CancelKnockback();
        return;
    }

    DirectX::XMFLOAT3 currentPosition = {};
    if (m_isStartFrame) {
        // 一番最初のフレームは、開始位置を設定する
        m_isStartFrame = false;
        currentPosition = m_startPosition;
    }
    else {
        m_moveElapsedTime += deltaTime;

        // 位置の更新
        currentPosition = m_transform->GetPosition();
        currentPosition = MiMath::Add(currentPosition, MiMath::Multiply(m_velocity, deltaTime));
        m_velocity.y += m_currentRequest.movementSource.gravity * deltaTime;
    }

    if (auto* bounds = StageBoundsControllerBehavior::Find(m_transform)) {
        const auto result = bounds->Resolve(m_transform, currentPosition);
        currentPosition = result.position;
        result.ClipVelocity(m_velocity);
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
                if (auto* bounds = StageBoundsControllerBehavior::Find(m_transform)) {
                    velocity = bounds->ConstrainVelocity(m_transform, m_rigidbody, velocity, deltaTime);
                }
                m_rigidbody->SetVelocity(velocity);
            }
            break;
        }
        default: break;
    }

    // === ノックバック終了判定 ===
    if (m_moveElapsedTime >= m_currentRequest.duration) {
        if (m_currentRequest.movementSource.mode == KnockbackMovementMode::SetRigidbodyVelocity && m_rigidbody) {
            DirectX::XMFLOAT3 stopVelocity = {};
            if (auto* bounds = StageBoundsControllerBehavior::Find(m_transform)) {
                stopVelocity = bounds->ConstrainVelocity(m_transform, m_rigidbody, stopVelocity, deltaTime);
            }
            m_rigidbody->SetVelocity(stopVelocity);
        }
        RestoreRigidbody();
        m_state = State::Inactive;
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

    // 既存のノックバックがある場合は、Rigidbody設定を復元してから最新のリクエストで置き換える。
    CancelKnockback();

    m_currentRequest = request;
    m_currentRequest.movementSource = movementSource;
    m_delayElapsedTime = 0.0f;
    m_moveElapsedTime = 0.0f;

    if (request.delay > 0.0f) m_state = State::WaitingDelay;
    else BeginMovement();
    return true;
}

void KnockbackReceiver::CancelKnockback()
{
    if (m_rigidbody && m_rigidbodyParametersSaved) m_rigidbody->SetVelocity({});
    RestoreRigidbody();
    m_state = State::Inactive;
    m_delayElapsedTime = 0.0f;
    m_moveElapsedTime = 0.0f;
    m_velocity = {};
    m_isStartFrame = true;
}

void KnockbackReceiver::BeginMovement()
{
    if (!m_transform) {
        CancelKnockback();
        return;
    }

    const KnockbackMovementSource& movementSource = m_currentRequest.movementSource;
    if (movementSource.mode == KnockbackMovementMode::SetRigidbodyVelocity) {
        if (!m_rigidbody) {
            CancelKnockback();
            return;
        }
        m_rbMass = m_rigidbody->GetMass();
        m_rbGravity = m_rigidbody->GetGravityScale();
        m_rbFriction = m_rigidbody->GetFriction();
        m_rigidbodyParametersSaved = true;
        m_rigidbody->SetGravityScale(0.0f);
        m_rigidbody->SetFriction({ 1.0f, 1.0f, 1.0f });
    }

    // Delay中に対象が移動しても現在位置から開始できるよう、ここで計算する。
    m_startPosition = m_currentRequest.overrideStartPosition
        ? m_currentRequest.startPosition
        : m_transform->GetPosition();
    const XMFLOAT3 targetPosition = EvaluateTargetPosition(m_startPosition, m_currentRequest);
    m_velocity = CalculateInitialVelocity(
        m_startPosition,
        targetPosition,
        movementSource.gravity,
        m_currentRequest.duration);

    m_moveElapsedTime = 0.0f;
    m_isStartFrame = true;
    m_state = State::Moving;
}

void KnockbackReceiver::RestoreRigidbody()
{
    if (!m_rigidbody || !m_rigidbodyParametersSaved) return;
    m_rigidbody->SetGravityScale(m_rbGravity);
    m_rigidbody->SetFriction(m_rbFriction);
    m_rigidbodyParametersSaved = false;
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
