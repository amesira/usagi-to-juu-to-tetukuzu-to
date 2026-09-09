#include "training_dummy_locomotion.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/rigidbody_component.h"

void TrainingDummyLocomotion::Initialize(GameObject* owner)
{
    m_rigidbody = owner->GetComponent<RigidbodyComponent>();
    if (m_rigidbody) m_rigidbody->SetIsKinematic(false);
}

void TrainingDummyLocomotion::Move(const DirectX::XMFLOAT3& desiredVelocity)
{
    if (!m_rigidbody) return;
    auto velocity = m_rigidbody->GetVelocity();
    velocity.x = desiredVelocity.x;
    velocity.z = desiredVelocity.z;
    m_rigidbody->SetVelocity(velocity);
}
