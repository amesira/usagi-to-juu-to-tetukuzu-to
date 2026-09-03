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
    TransformComponent* m_transform = nullptr;
    RigidbodyComponent* m_rigidbody = nullptr;

    KnockbackRequest m_currentRequest;

    DirectX::XMFLOAT3 m_startPosition = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_velocity = { 0.0f, 0.0f, 0.0f };
    float m_elapsedTime = 0.0f;
    bool m_isActive = false;

    bool m_isStartFrame = true;

    // 一時保持するRigidbodyのパラメータ
    float m_rbMass = 1.0f;
    float m_rbGravity = -9.81f;
    DirectX::XMFLOAT3 m_rbFriction = { 0.0f, 0.0f, 0.0f };

    // デフォルトとなるノックバック移動方法
    KnockbackMovementSource m_defaultMovementSource = {
        KnockbackMovementMode::SetTransformPosition,
        true,
        -9.81f
    };

public:
    void Initialize(TransformComponent* transform, RigidbodyComponent* rigidbody);
    void Update(float deltaTime);

    bool StartKnockback(const KnockbackRequest& request);
    void CancelKnockback();

    bool IsActive() const { return m_isActive; }
    const KnockbackRequest& GetCurrentRequest() const { return m_currentRequest; }

    /// @brief デフォルトのノックバック移動方法を設定する
    void SetDefaultMovementSource(const KnockbackMovementSource& movementSource) {
        m_defaultMovementSource = movementSource;
    }

private:
    /// @brief ノックバックの初速度を計算する
    DirectX::XMFLOAT3 CalculateInitialVelocity(
        const DirectX::XMFLOAT3& startPosition,
        const DirectX::XMFLOAT3& targetPosition,
        float gravity,
        float duration) const;

    /// @brief ノックバックの目標位置を評価する
    DirectX::XMFLOAT3 EvaluateTargetPosition(
        const DirectX::XMFLOAT3& startPosition,
        const KnockbackRequest& request) const;

};
