#include "enemy_attack_combat.h"

#include <cmath>

#include "Game/ActorBehavior/Enemy/P00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/P10_Locomotion/enemy_locomotion_controller.h"
#include "Engine/Component/transform_component.h"

bool EnemyAttackCombat::CanStart(const EnemyContext& context) const
{
    if (!context.runtimeState.hasCombatTarget || !context.transform) return false;
    const auto position = context.transform->GetPosition();
    const auto target = context.runtimeState.combatTargetPosition;
    return std::hypot(target.x - position.x, target.z - position.z) <= m_attackDistance;
}

void EnemyAttackCombat::Start(EnemyContext& context)
{
    EnemyLocomotionController::LocomotionRequest request;
    request.priority = 20;
    request.canMove = false;
    request.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    request.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    m_locomotionRequestHandle = context.locomotionController
        ? context.locomotionController->AddRequest(request)
        : EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

EnemyCombatStatus EnemyAttackCombat::Update(EnemyContext& context, float deltaTime)
{
    if (!CanStart(context)) return EnemyCombatStatus::Success;

    if (context.locomotionController) {
        EnemyLocomotionController::LocomotionRequest request;
        request.priority = 20;
        request.canMove = false;
        request.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
        request.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
        context.locomotionController->UpdateRequest(m_locomotionRequestHandle, request);
    }

    UpdateAttack(context, deltaTime);
    return EnemyCombatStatus::Running;
}

void EnemyAttackCombat::Finish(EnemyContext& context)
{
    Abort(context);
}

void EnemyAttackCombat::Abort(EnemyContext& context)
{
    if (context.locomotionController) {
        context.locomotionController->RemoveRequest(m_locomotionRequestHandle);
    }
    m_locomotionRequestHandle = EnemyLocomotionController::INVALID_REQUEST_HANDLE;
}

void EnemyAttackCombat::UpdateAttack(EnemyContext&, float)
{
    // TODO: 攻撃の選択、予備動作、攻撃判定、後隙を敵種ごとに実装する。
}
