#include "enemy_move_behavior.h"

#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/transform_component.h"
#include "Utility/mi_math.h"

void EnemyMoveBehavior::Initialize(EnemyContext& context)
{
    if (context.rigidbody) context.rigidbody->SetIsKinematic(false);
}

void EnemyMoveBehavior::Finalize()
{
}

void EnemyMoveBehavior::UpdateMove(
    EnemyContext& context,
    const EnemyMoveIntent& intent,
    float deltaTime)
{
    if (!context.transform || !context.rigidbody) return;

    DirectX::XMFLOAT3 velocity = context.rigidbody->GetVelocity();
    if (intent.canMove) {
        velocity.x = intent.moveDirection.x * intent.moveSpeed;
        velocity.z = intent.moveDirection.z * intent.moveSpeed;
    }
    else {
        velocity.x = 0.0f;
        velocity.z = 0.0f;
    }
    context.rigidbody->SetVelocity(velocity);

    if (intent.canRotate && MiMath::Length(intent.rotateDirection) > 0.001f) {
        const DirectX::XMFLOAT4 targetRotation = MiMath::LookRotation(
            intent.rotateDirection, { 0.0f, 1.0f, 0.0f });
        context.transform->SetRotation(MiMath::Slerp(
            context.transform->GetRotation(), targetRotation,
            intent.rotationSpeed * deltaTime));
    }

    context.runtimeState.controlVelocity = { velocity.x, 0.0f, velocity.z };
    context.runtimeState.isGrounded = context.rigidbody->GetIsGrounded();
}
