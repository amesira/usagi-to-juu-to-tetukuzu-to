//---------------------------------------------------
// File  ：_/Base/HitReceiver/knockback_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・ノックバックの処理をまとめるクラス
//---------------------------------------------------
#pragma once
#include "hit_receiver_context.h"

class TransformComponent;
class RigidbodyComponent;

class KnockbackReceiver {
private:
    enum class State {
        Inactive,
        WaitingDelay,
        Moving,
    };

    // ・オンの場合、ノックバックの開始を無効化する
    // ・リクエスト構造体のenableよりも強い制約を持つ
    bool m_isLockKnockback = false;

    TransformComponent* m_transform = nullptr;
    RigidbodyComponent* m_rigidbody = nullptr;

    HitReceiver::KnockbackRequest m_currentRequest;

    DirectX::XMFLOAT3 m_startPosition = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_velocity = { 0.0f, 0.0f, 0.0f };
    float m_delayElapsedTime = 0.0f;
    float m_moveElapsedTime = 0.0f;
    State m_state = State::Inactive;

    bool m_isStartFrame = true;

    // 一時保持するRigidbodyのパラメータ
    float m_rbMass = 1.0f;
    float m_rbGravity = -9.81f;
    DirectX::XMFLOAT3 m_rbFriction = { 0.0f, 0.0f, 0.0f };
    bool m_rigidbodyParametersSaved = false;

    // デフォルトとなるノックバック移動方法
    HitReceiver::KnockbackMovementSource m_defaultMovementSource = {
        HitReceiver::KnockbackMovementMode::SetTransformPosition,
        -9.81f
    };

public:
    void Initialize(TransformComponent* transform, RigidbodyComponent* rigidbody);
    void Update(float deltaTime, float unscaledDeltaTime);

    void SetLockKnockback(bool isLock) { m_isLockKnockback = isLock; }

    bool StartKnockback(const HitReceiver::KnockbackRequest& request);
    void CancelKnockback();

    bool IsActive() const { return m_state != State::Inactive; }
    bool IsPending() const { return m_state == State::WaitingDelay; }
    bool IsMoving() const { return m_state == State::Moving; }
    const HitReceiver::KnockbackRequest& GetCurrentRequest() const { return m_currentRequest; }

    /// @brief デフォルトのノックバック移動方法を設定する
    void SetDefaultMovementSource(const HitReceiver::KnockbackMovementSource& movementSource) {
        m_defaultMovementSource = movementSource;
    }

private:
    void BeginMovement();
    void UpdateDelay(float unscaledDeltaTime);
    void UpdateMovement(float deltaTime);
    void RestoreRigidbody();

    /// @brief ノックバックの初速度を計算する
    DirectX::XMFLOAT3 CalculateInitialVelocity(
        const DirectX::XMFLOAT3& startPosition,
        const DirectX::XMFLOAT3& targetPosition,
        float gravity,
        float duration) const;

    /// @brief ノックバックの目標位置を評価する
    DirectX::XMFLOAT3 EvaluateTargetPosition(
        const DirectX::XMFLOAT3& startPosition,
        const HitReceiver::KnockbackRequest& request) const;

};
