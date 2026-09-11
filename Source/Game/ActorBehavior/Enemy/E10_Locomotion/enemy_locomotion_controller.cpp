//===================================================
// File  ：_/E10_Locomotion/enemy_locomotion_controller.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_locomotion_controller.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Engine/Component/transform_component.h"
#include "Utility/mi_math.h"

using namespace DirectX;

namespace {
    /// @brief 要求の方向ソース情報から、ワールド座標系での正規化された方向ベクトルを計算する
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

/// @brief 複数の移動要求から優先度の高いものを選択し、EnemyMoveIntentを構築する
EnemyMoveIntent EnemyLocomotionController::BuildIntent(const EnemyContext& context)
{
    const LocomotionRequest* selected = nullptr;
    for (const RequestSlot& slot : m_requests) {
        if (!slot.active) continue;
        if (!selected || slot.request.priority > selected->priority) {
            selected = &slot.request;
        }
    }

    EnemyMoveIntent intent;
    if (selected) {
        intent.moveDirection = CalculateDirection(selected->moveDirection, context);
        intent.rotateDirection = CalculateDirection(selected->rotateDirection, context);
        intent.moveSpeedMultiplier = selected->moveSpeedMultiplier;
        intent.rotationSpeedMultiplier = selected->rotationSpeedMultiplier;
        intent.movementMode = selected->movementMode;
        intent.canRotate = selected->canRotate;
        intent.canMove = selected->canMove;
        intent.useGravity = selected->useGravity;
    }
    // Stun/Deadなど、より高優先度の通常要求がある場合は強制移動を抑止する。
    if (m_hasForceMoveRequest && (!selected || m_forceMoveRequest.priority >= selected->priority)) {
        intent.forceMoveIntent.isActive = true;
        intent.forceMoveIntent.targetPosition = m_forceMoveRequest.targetPosition;
    }
    m_forceMoveRequest = {};
    m_hasForceMoveRequest = false;
    return intent;
}
