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
    float m_elapsedTime = 0.0f;
    bool m_isActive = false;

public:
    void Initialize(TransformComponent* transform, RigidbodyComponent* rigidbody);
    void Update(float deltaTime);

    bool StartKnockback(const KnockbackRequest& request);
    void CancelKnockback();

    bool IsActive() const { return m_isActive; }
    float GetProgress() const;
    const KnockbackRequest& GetCurrentRequest() const { return m_currentRequest; }

private:
    DirectX::XMFLOAT3 CalculateTargetPosition() const;
    float EvaluateEasing(float rate) const;
};
