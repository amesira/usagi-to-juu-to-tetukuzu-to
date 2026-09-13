//===================================================
// File  ：_/DualPistols/player_dual_pistols_firing.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_firing.h"
#include "player_dual_pistols_context.h"

#include "Engine/Component/transform_component.h"
#include "Game/Factory/projectile_factory.h"
#include "Utility/mi_math.h"

#include "Game/ActorBehavior/Player/P40_Weapon/player_weapon_controller.h"

namespace {
    constexpr CollisionLayerMask DUAL_PISTOLS_HIT_LAYER_MASK =
        COLLISION_LAYER_MASK_ALL &
        ~CollisionLayerToMask(CollisionLayer::Player) &
        ~CollisionLayerToMask(CollisionLayer::Bullet);
}

void PlayerDualPistolsFiring::Initialize(PlayerDualPistolsContext& context)
{
    
}

void PlayerDualPistolsFiring::Fire(
    PlayerDualPistolsContext& context,
    const FireRequest& request)
{
    if (!context.scene) return;
    if (context.weaponController) {
        if (!context.weaponController->CanConsume(1)) {
            return; // 弾薬が足りない場合は発射しない
        }

        context.weaponController->TryConsume(1);
    }

    const auto& settings = context.settings();
    const DirectX::XMFLOAT3 fireDirection = MiMath::Normalize(request.fireDirection);

    ProjectileFactory::BulletCreateDesc bulletDesc;
    bulletDesc.position = MiMath::Add(
        request.muzzlePosition,
        MiMath::Multiply(fireDirection, settings.bulletSpawnForwardOffset));
    bulletDesc.velocity = MiMath::Multiply(fireDirection, settings.bulletSpeed);
    bulletDesc.radius = settings.bulletRadius;
    bulletDesc.lifeTime = settings.bulletLifetime;
    bulletDesc.layerMask = DUAL_PISTOLS_HIT_LAYER_MASK;
    bulletDesc.attacker = context.playerTransform
        ? context.playerTransform->GetOwner() : nullptr;
    bulletDesc.damage = settings.bulletDamage;

    ProjectileFactory::CreateBullet(context.scene, bulletDesc);
}
