#include "attached_effect_handle.h"

#include "Engine/Core/game_object.h"
#include "Game/ActorBehavior/transform_constraint_behavior.h"

namespace {
    TransformConstraintBehavior* GetConstraint(const EffectHandle& effect)
    {
        GameObject* gameObject = effect.GetGameObject();
        if (!gameObject) return nullptr;
        return gameObject->GetComponent<TransformConstraintBehavior>();
    }
}

void AttachedEffectHandle::SetLocalTransform(const EffectTransform& transform)
{
    TransformConstraintBehavior* constraint = GetConstraint(m_effect);
    if (!constraint) return;

    constraint->SetLocalPosition(transform.position);
    constraint->SetLocalRotation(transform.rotation);
    constraint->SetLocalScaling(transform.scaling);
    constraint->ApplyConstraint();
}

void AttachedEffectHandle::SetLocalPosition(const DirectX::XMFLOAT3& position)
{
    if (TransformConstraintBehavior* constraint = GetConstraint(m_effect)) {
        constraint->SetLocalPosition(position);
        constraint->ApplyConstraint();
    }
}

void AttachedEffectHandle::SetLocalRotation(const DirectX::XMFLOAT4& rotation)
{
    if (TransformConstraintBehavior* constraint = GetConstraint(m_effect)) {
        constraint->SetLocalRotation(rotation);
        constraint->ApplyConstraint();
    }
}

void AttachedEffectHandle::SetLocalScaling(const DirectX::XMFLOAT3& scaling)
{
    if (TransformConstraintBehavior* constraint = GetConstraint(m_effect)) {
        constraint->SetLocalScaling(scaling);
        constraint->ApplyConstraint();
    }
}
