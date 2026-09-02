//===================================================
// File  ：_/Base/HitReceiver/knockback_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "knockback_receiver.h"

void KnockbackReceiver::Initialize(TransformComponent* transform, RigidbodyComponent* rigidbody)
{
    m_transform = transform;
    m_rigidbody = rigidbody;
}

void KnockbackReceiver::Update(float)
{
}

bool KnockbackReceiver::StartKnockback(const KnockbackRequest&)
{
    return false;
}

void KnockbackReceiver::CancelKnockback()
{
}

float KnockbackReceiver::GetProgress() const
{
    return 0.0f;
}

DirectX::XMFLOAT3 KnockbackReceiver::CalculateTargetPosition() const
{
    return m_startPosition;
}

float KnockbackReceiver::EvaluateEasing(float rate) const
{
    return rate;
}
