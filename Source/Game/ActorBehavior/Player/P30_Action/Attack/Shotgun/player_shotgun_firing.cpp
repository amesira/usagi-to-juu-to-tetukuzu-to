//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_firing.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの発射処理を制御するクラス
//---------------------------------------------------
#include "player_shotgun_firing.h"
#include "player_shotgun_context.h"

#include "Engine/Component/transform_component.h"
#include "Game/Factory/projectile_factory.h"
#include "Utility/mi_math.h"

namespace
{
    constexpr CollisionLayerMask SHOTGUN_HIT_LAYER_MASK =
        COLLISION_LAYER_MASK_ALL &
        ~CollisionLayerToMask(CollisionLayer::Player) &
        ~CollisionLayerToMask(CollisionLayer::Bullet);
}

void PlayerShotgunFiring::Fire(PlayerShotgunContext& context, const FireRequest& request)
{
    if (!context.scene) return;

    const float chargeRate = MiMath::Clamp(request.chargeRate, 0.0f, 1.0f);
    const auto& settings = context.settings();
    const float bulletSpeed = MiMath::Lerp(settings.minBulletSpeed, settings.maxBulletSpeed, chargeRate);

    ProjectileFactory::BulletCreateDesc bulletDesc;
    bulletDesc.position = MiMath::Add(
        request.muzzlePosition,
        MiMath::Multiply(request.fireDirection, settings.bulletSpawnForwardOffset));
    bulletDesc.velocity = MiMath::Multiply(
        MiMath::Normalize(request.fireDirection),
        bulletSpeed);
    bulletDesc.radius = MiMath::Lerp(settings.minBulletRadius, settings.maxBulletRadius, chargeRate);
    bulletDesc.lifeTime = settings.bulletLifetime;
    bulletDesc.layerMask = SHOTGUN_HIT_LAYER_MASK;
    bulletDesc.attacker = context.playerTransform
        ? context.playerTransform->GetOwner() : nullptr;
    bulletDesc.damage = bulletDesc.radius * 20.0f;

    ProjectileFactory::CreateBullet(context.scene, bulletDesc);

    context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::Fire);
}
