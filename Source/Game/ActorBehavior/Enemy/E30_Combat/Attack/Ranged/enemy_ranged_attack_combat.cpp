//===================================================
// File  ：_/E_30_Combat/Attack/Ranged/enemy_ranged_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_ranged_attack_combat.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Game/ActorBehavior/Enemy/enemy_animation_controller.h"
#include <Windows.h>
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Engine/Graphics/model_animation_utility.h"
#include "Game/Factory/projectile_factory.h"
#include "Utility/mi_math.h"

#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"

namespace {
    constexpr CollisionLayerMask ENEMY_BULLET_HIT_LAYER_MASK =
        COLLISION_LAYER_MASK_ALL
        & ~CollisionLayerToMask(CollisionLayer::Enemy)
        & ~CollisionLayerToMask(CollisionLayer::Bullet);
}

void EnemyRangedAttackCombat::Initialize(EnemyContext& context)
{
    EnemyAttackCombat::Initialize(context);

    GameObject* owner = context.owner ? context.owner->GetOwner() : nullptr;
    m_model = owner ? owner->GetComponent<ModelComponent>() : nullptr;
    if (!m_model) return;

    ModelAnimationUtility::FindBoneIndex(
        *m_model,
        settings().leftMuzzleBoneName,
        m_leftMuzzleBoneIndex);
    ModelAnimationUtility::FindBoneIndex(
        *m_model,
        settings().rightMuzzleBoneName,
        m_rightMuzzleBoneIndex);
    if (m_leftMuzzleBoneIndex == static_cast<unsigned int>(-1)
        || m_rightMuzzleBoneIndex == static_cast<unsigned int>(-1))
        OutputDebugStringA("Enemy ranged attack: muzzle bone not found; check Gun.L / Gun.R.\n");
}

void EnemyRangedAttackCombat::BeginAttack(EnemyContext& context)
{
    m_shotsFired = 0;
    m_timeUntilNextShot = settings().firstShotDelay;
    m_waitingForFire = true;
    if (context.animationController) {
        context.animationController->PlayCombatAnimation(EnemyAnimationController::Animation::Shot, settings().shotPlaybackSpeed);
    }

    m_locomotionRequest.priority = 50;
    m_locomotionRequest.canMove = false;
    m_locomotionRequest.canRotate = true;
    m_locomotionRequest.rotateDirection.source = EnemyLocomotionController::DirectionSource::TargetPosition;
    m_locomotionRequest.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    m_locomotionRequestId = context.locomotionController->AddRequest(m_locomotionRequest);
}

EnemyCombatStatus EnemyRangedAttackCombat::UpdateAttack(EnemyContext& context, float deltaTime)
{
    m_timeUntilNextShot -= deltaTime;
    // アニメーションを実際に評価する機会を確保するため、1フレーム1イベントまで。
    if (m_shotsFired < settings().shotCount && m_timeUntilNextShot <= 0.0f) {
        if (!m_waitingForFire) {
            if (context.animationController)
                context.animationController->PlayCombatAnimation(EnemyAnimationController::Animation::Shot, settings().shotPlaybackSpeed);
            m_waitingForFire = true;
            m_timeUntilNextShot = settings().firstShotDelay;
            return EnemyCombatStatus::Running;
        }
        if (m_leftMuzzleBoneIndex == static_cast<unsigned int>(-1)
            || m_rightMuzzleBoneIndex == static_cast<unsigned int>(-1)) return EnemyCombatStatus::Failure;
        SetAimPosition(context.runtimeState.combatTargetPosition);
        FireShot(context, GetAimPosition());
        ++m_shotsFired;
        m_waitingForFire = false;
        m_timeUntilNextShot = (std::max)(0.0f, settings().shotInterval - settings().firstShotDelay);
    }

    // 移動要求更新
    m_locomotionRequest.rotateDirection.targetPosition = context.runtimeState.combatTargetPosition;
    context.locomotionController->UpdateRequest(m_locomotionRequestId, m_locomotionRequest);

    return m_shotsFired >= settings().shotCount
        ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
}

void EnemyRangedAttackCombat::EndAttack(EnemyContext& context)
{
    if (context.animationController) {
        context.animationController->StopCombatAnimation(settings().recoveryDuration);
    }

    ClearAttackEffects(context);
    m_timeUntilNextShot = 0.0f;

    if (m_locomotionRequestId != -1) {
        context.locomotionController->RemoveRequest(m_locomotionRequestId);
        m_locomotionRequestId = -1;
    }
}

void EnemyRangedAttackCombat::FireShot(
    EnemyContext& context,
    const DirectX::XMFLOAT3& targetPosition)
{
    FireFromBone(context, m_leftMuzzleBoneIndex, targetPosition);
    FireFromBone(context, m_rightMuzzleBoneIndex, targetPosition);

    // 効果音再生
    Game::Audio()->PlaySe(GameSe::EnemyShot);
}

void EnemyRangedAttackCombat::Cancel(EnemyContext& context)
{
    EnemyAttackCombat::Cancel(context);
    if (context.animationController) context.animationController->StopCombatAnimation();
    m_waitingForFire = false;
}

void EnemyRangedAttackCombat::ClearAttackEffects(EnemyContext&)
{
    // TODO: 射撃演出・要求を解除する。発射済みの弾は弾側が寿命を管理する。
}

bool EnemyRangedAttackCombat::FireFromBone(
    EnemyContext& context,
    unsigned int boneIndex,
    const DirectX::XMFLOAT3& targetPosition)
{
    if (!context.scene || !context.transform || !m_model
        || boneIndex == static_cast<unsigned int>(-1)) {
        return false;
    }

    ModelAnimationUtility::BoneTransform muzzle;
    if (!ModelAnimationUtility::GetBoneWorldTransform(
            *m_model, *context.transform, boneIndex, muzzle)) {
        return false;
    }

    DirectX::XMFLOAT3 aimPosition = targetPosition;
    aimPosition.y += settings().targetHeightOffset;
    const DirectX::XMFLOAT3 fireDirection = MiMath::Normalize(
        MiMath::Subtract(aimPosition, muzzle.position));

    ProjectileFactory::BulletCreateDesc bulletDesc;
    bulletDesc.position = MiMath::Add(
        muzzle.position,
        MiMath::Multiply(
            fireDirection,
            settings().projectileSpawnForwardOffset));
    bulletDesc.velocity =
        MiMath::Multiply(fireDirection, settings().projectileSpeed);
    bulletDesc.radius = settings().projectileRadius;
    bulletDesc.lifeTime = settings().projectileLifeTime;
    bulletDesc.layerMask = ENEMY_BULLET_HIT_LAYER_MASK;
    bulletDesc.attacker = context.owner ? context.owner->GetOwner() : nullptr;
    bulletDesc.damage = settings().projectileDamage;

    return ProjectileFactory::CreateBullet(context.scene, bulletDesc) != nullptr;
}
