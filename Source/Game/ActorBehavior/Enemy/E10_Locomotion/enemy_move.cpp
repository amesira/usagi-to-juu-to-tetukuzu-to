//===================================================
// File  ：_/E10_Locomotion/enemy_move.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_move.h"

#include <algorithm>

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/transform_component.h"
#include "Utility/mi_math.h"

void EnemyMove::Initialize(EnemyContext& context, const EnemyMoveSettingsAsset* settingsAsset)
{
    GameObject* owner = context.transform ? context.transform->GetOwner() : nullptr;
    if (owner == nullptr) return;

    m_context.owner = owner;
    m_context.scene = context.scene;
    m_context.transform = context.transform;
    m_context.rigidbody = context.rigidbody;
    m_context.collider = owner ? owner->GetComponent<CapsuleColliderComponent>() : nullptr;
    m_context.settingsAsset = settingsAsset;
    m_context.runtimeState = {};

    if (m_context.rigidbody) m_context.rigidbody->SetIsKinematic(false);
    m_motor.Reset();
    m_pathFollower.Initialize(m_context.settings().waypointReachDistance);
    m_effects.Initialize(m_context);
    context.pathFollower = &m_pathFollower;
}

void EnemyMove::Finalize()
{
    m_effects.Finalize();
    m_pathFollower.ClearPath();
    m_motor.Reset();
    m_context = {};
}

void EnemyMove::UpdateMove(EnemyContext& context, const EnemyMoveIntent& intent, float deltaTime)
{
    if (!m_context.transform || !m_context.rigidbody) return;

    m_context.runtimeState.wasGrounded = m_context.runtimeState.isGrounded;
    m_context.runtimeState.isGrounded = m_context.rigidbody->GetIsGrounded();
    m_motor.UpdateMotor(m_context, intent, deltaTime);

    if (intent.movementMode != EnemyMovementMode::KeepRigidbodyVelocity) {
        DirectX::XMFLOAT3 velocity = m_context.rigidbody->GetVelocity();
        velocity.x = m_context.runtimeState.controlVelocity.x;
        velocity.z = m_context.runtimeState.controlVelocity.z;
        m_context.rigidbody->SetVelocity(velocity);
    }

    if (intent.canRotate && MiMath::Length(intent.rotateDirection) > 0.001f) {
        const DirectX::XMFLOAT4 targetRotation = MiMath::LookRotation(
            intent.rotateDirection, { 0.0f, 1.0f, 0.0f });
        const float interpolation = (std::min)(
            1.0f,
            m_context.settings().rotationSpeed
                * intent.rotationSpeedMultiplier * deltaTime);
        m_context.transform->SetRotation(MiMath::Slerp(
            m_context.transform->GetRotation(), targetRotation, interpolation));
    }

    m_effects.Update(m_context);

    context.runtimeState.controlVelocity = m_context.runtimeState.controlVelocity;
    context.runtimeState.isGrounded = m_context.runtimeState.isGrounded;
}
