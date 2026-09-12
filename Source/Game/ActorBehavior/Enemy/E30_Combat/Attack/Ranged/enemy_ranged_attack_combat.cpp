//===================================================
// File  ：_/E_30_Combat/Attack/Ranged/enemy_ranged_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_ranged_attack_combat.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/enemy_behavior.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Engine/Graphics/model_animation_utility.h"
#include "Game/Factory/projectile_factory.h"
#include "Utility/mi_math.h"

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
}

void EnemyRangedAttackCombat::BeginAttack(EnemyContext&)
{
    m_shotsFired = 0;
    m_timeUntilNextShot = 0.0f;
}

EnemyCombatStatus EnemyRangedAttackCombat::UpdateAttack(EnemyContext& context, float deltaTime)
{
    m_timeUntilNextShot -= deltaTime;
    // 長いフレームでも指定回数を超えず、発射間隔の余りを維持する。
    while (m_shotsFired < settings().shotCount && m_timeUntilNextShot <= 0.0f) {
        SetAimPosition(context.runtimeState.combatTargetPosition);
        FireShot(context, GetAimPosition());
        ++m_shotsFired;
        m_timeUntilNextShot += settings().shotInterval;
    }
    return m_shotsFired >= settings().shotCount
        ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
}

void EnemyRangedAttackCombat::EndAttack(EnemyContext& context)
{
    ClearAttackEffects(context);
    m_timeUntilNextShot = 0.0f;
}

void EnemyRangedAttackCombat::FireShot(
    EnemyContext& context,
    const DirectX::XMFLOAT3& targetPosition)
{
    FireFromBone(context, m_leftMuzzleBoneIndex, targetPosition);
    FireFromBone(context, m_rightMuzzleBoneIndex, targetPosition);
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
    bulletDesc.materialName = "EnemyBulletHologramMaterial";

    return ProjectileFactory::CreateBullet(context.scene, bulletDesc) != nullptr;
}
