//===================================================
// File  ：_/DualPistols/player_dual_pistols_aim.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_aim.h"
#include "player_dual_pistols_context.h"

#include "Engine/Component/camera_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_types.h"

#include "Game/ActorBehavior/Player/player_animation_controller.h"

#include "Utility/mi_math.h"

namespace {
    constexpr CollisionLayerMask AIM_LAYER_MASK =
        COLLISION_LAYER_MASK_ALL &
        ~CollisionLayerToMask(CollisionLayer::Player) &
        ~CollisionLayerToMask(CollisionLayer::Bullet);
}

void PlayerDualPistolsAim::Initialize(PlayerDualPistolsContext& context)
{
}

void PlayerDualPistolsAim::EnterAim(PlayerDualPistolsContext& context)
{
}

void PlayerDualPistolsAim::UpdateAim(PlayerDualPistolsContext& context, float deltaTime)
{
    m_aimResult = {};
    if (!context.scene || !context.cameraTransform || !context.cameraComponent) {
        return;
    }

    m_aimResult.cameraRayOrigin = context.cameraTransform->GetPosition();
    m_aimResult.cameraRayDirection = MiMath::Normalize(context.cameraComponent->GetForward());

    RaycastHit hit;
    m_aimResult.hasTargetHit = CollisionQuery::Raycast(
        context.scene,
        hit,
        m_aimResult.cameraRayOrigin,
        m_aimResult.cameraRayDirection,
        context.settings().aimMaxDistance,
        AIM_LAYER_MASK);

    if (m_aimResult.hasTargetHit) {
        m_aimResult.targetPosition = hit.hitPoint;
    }
    else {
        m_aimResult.targetPosition = MiMath::Add(
            m_aimResult.cameraRayOrigin,
            MiMath::Multiply(m_aimResult.cameraRayDirection, context.settings().aimMaxDistance));
    }

    const PlayerMuzzleState& leftMuzzle = context.runtimeState.leftMuzzle;
    m_aimResult.hasLeftMuzzle = leftMuzzle.isValid;
    if (m_aimResult.hasLeftMuzzle) {
        m_aimResult.leftMuzzlePosition = leftMuzzle.position;
        m_aimResult.leftFireDirection = MiMath::Normalize(MiMath::Subtract(
            m_aimResult.targetPosition,
            m_aimResult.leftMuzzlePosition));
    }

    const PlayerMuzzleState& rightMuzzle = context.runtimeState.rightMuzzle;
    m_aimResult.hasRightMuzzle = rightMuzzle.isValid;
    if (m_aimResult.hasRightMuzzle) {
        m_aimResult.rightMuzzlePosition = rightMuzzle.position;
        m_aimResult.rightFireDirection = MiMath::Normalize(MiMath::Subtract(
            m_aimResult.targetPosition,
            m_aimResult.rightMuzzlePosition));
    }
}

void PlayerDualPistolsAim::ExitAim(PlayerDualPistolsContext& context)
{

}
