#include "enemy_locomotion_controller.h"

#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"
#include "Engine/Component/transform_component.h"
#include "Utility/mi_math.h"

using namespace DirectX;

namespace {
XMFLOAT3 CalculateDirection(
    const EnemyLocomotionController::DirectionSourceInfo& source,
    const EnemyContext& context)
{
    XMFLOAT3 direction = {};
    switch (source.source) {
    case EnemyLocomotionController::DirectionSource::FixedDirection:
        direction = source.fixedDirection;
        break;
    case EnemyLocomotionController::DirectionSource::TargetPosition:
        if (context.transform) {
            direction = MiMath::Subtract(source.targetPosition, context.transform->GetPosition());
        }
        break;
    case EnemyLocomotionController::DirectionSource::None:
    default:
        break;
    }

    direction.y = 0.0f;
    if (MiMath::Length(direction) <= 0.001f) return {};
    return MiMath::Normalize(direction);
}
}

void EnemyLocomotionController::Initialize()
{
    ClearRequests();
}

void EnemyLocomotionController::Finalize()
{
    ClearRequests();
}

int EnemyLocomotionController::AddRequest(const LocomotionRequest& request)
{
    for (int i = 0; i < MAX_REQUEST_COUNT; ++i) {
        if (m_requests[i].active) continue;
        m_requests[i].active = true;
        m_requests[i].request = request;
        return i;
    }
    return INVALID_REQUEST_HANDLE;
}

bool EnemyLocomotionController::UpdateRequest(int handle, const LocomotionRequest& request)
{
    if (handle < 0 || handle >= MAX_REQUEST_COUNT || !m_requests[handle].active) return false;
    m_requests[handle].request = request;
    return true;
}

void EnemyLocomotionController::RemoveRequest(int handle)
{
    if (handle < 0 || handle >= MAX_REQUEST_COUNT) return;
    m_requests[handle] = {};
}

void EnemyLocomotionController::ClearRequests()
{
    m_requests = {};
}

EnemyMoveIntent EnemyLocomotionController::BuildIntent(const EnemyContext& context) const
{
    const LocomotionRequest* selected = nullptr;
    for (const RequestSlot& slot : m_requests) {
        if (!slot.active) continue;
        if (!selected || slot.request.priority > selected->priority) {
            selected = &slot.request;
        }
    }

    EnemyMoveIntent intent;
    if (!selected) return intent; // 要求がないときは停止する。

    intent.moveDirection = CalculateDirection(selected->moveDirection, context);
    intent.rotateDirection = CalculateDirection(selected->rotateDirection, context);
    intent.moveSpeed = selected->moveSpeed;
    intent.rotationSpeed = selected->rotationSpeed;
    intent.canMove = selected->canMove;
    intent.canRotate = selected->canRotate;
    return intent;
}
