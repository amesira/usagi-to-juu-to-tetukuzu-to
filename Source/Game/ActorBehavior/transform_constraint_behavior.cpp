//===================================================
// transform_constraint_behavior.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/06/02
//===================================================
#include "transform_constraint_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"

#include "Engine/Editor/BaseEditor/inspector_view_window.h"

#include "Utility/mi_math.h"

void TransformConstraintBehavior::Start()
{
    m_transform = GetOwner()->GetComponent<TransformComponent>();
    ApplyConstraint();
}

void TransformConstraintBehavior::Update()
{
    ApplyConstraint();
}

void TransformConstraintBehavior::ApplyConstraint()
{
    if (!m_transform && GetOwner()) {
        m_transform = GetOwner()->GetComponent<TransformComponent>();
    }
    if (!m_transform || !m_target) return;
    if (!m_target->GetEnable()) return;

    m_transform->SetPosition(CalculateConstrainedPosition());
    m_transform->SetRotation(CalculateConstrainedRotation());
    m_transform->SetScaling(CalculateConstrainedScaling());
}

void TransformConstraintBehavior::DrawComponentInspector()
{
    ImGui::DragFloat3("Local Position", &m_localPosition.x, 0.05f);
    ImGui::DragFloat4("Local Rotation", &m_localRotation.x, 0.01f);
    ImGui::DragFloat3("Local Scaling", &m_localScaling.x, 0.05f);
    ImGui::Checkbox("Consider Rotation", &m_considerRotation);
    ImGui::Checkbox("Consider Scaling", &m_considerScaling);
    ApplyConstraint();
}

XMFLOAT3 TransformConstraintBehavior::CalculateConstrainedPosition() const
{
    XMFLOAT3 offset = m_localPosition;

    // オフセットにスケーリングを考慮する場合は、対象のスケーリングを掛ける
    if (m_considerScaling) {
        const XMFLOAT3 targetScale = m_target->GetScaling();
        offset.x *= targetScale.x;
        offset.y *= targetScale.y;
        offset.z *= targetScale.z;
    }

    const XMFLOAT3 targetPosition = m_target->GetPosition();
    if (!m_considerRotation) {
        return MiMath::Add(targetPosition, offset);
    }

    // オフセットに回転を考慮する場合は、対象の回転に合わせてオフセットを回転させる
    XMFLOAT3 rotatedOffset = { 0.0f, 0.0f, 0.0f };
    rotatedOffset = MiMath::Add(rotatedOffset, MiMath::Multiply(m_target->GetRight(), offset.x));
    rotatedOffset = MiMath::Add(rotatedOffset, MiMath::Multiply(m_target->GetUp(), offset.y));
    rotatedOffset = MiMath::Add(rotatedOffset, MiMath::Multiply(m_target->GetForward(), offset.z));

    return MiMath::Add(targetPosition, rotatedOffset);
}

XMFLOAT4 TransformConstraintBehavior::CalculateConstrainedRotation() const
{
    XMVECTOR localRotation = XMQuaternionNormalize(XMLoadFloat4(&m_localRotation));
    if (!m_considerRotation) {
        XMFLOAT4 result;
        XMStoreFloat4(&result, localRotation);
        return result;
    }

    XMVECTOR targetRotation = XMQuaternionNormalize(m_target->GetRotationVector());
    XMFLOAT4 result;
    XMStoreFloat4(
        &result,
        XMQuaternionNormalize(XMQuaternionMultiply(localRotation, targetRotation)));
    return result;
}

XMFLOAT3 TransformConstraintBehavior::CalculateConstrainedScaling() const
{
    if (!m_considerScaling) return m_localScaling;

    const XMFLOAT3 targetScaling = m_target->GetScaling();
    return {
        m_localScaling.x * targetScaling.x,
        m_localScaling.y * targetScaling.y,
        m_localScaling.z * targetScaling.z,
    };
}
