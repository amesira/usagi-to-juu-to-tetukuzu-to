//===================================================
// File  ：_/DualPistols/player_dual_pistols_firing.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_firing.h"
#include "player_dual_pistols_context.h"

#include "Game/Factory/projectile_factory.h"
#include "Utility/mi_math.h"

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

    ProjectileFactory::CreateBullet(context.scene, bulletDesc);
}
