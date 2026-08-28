//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_firing.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの発射処理を制御するクラス
//---------------------------------------------------
#include "player_shotgun_firing.h"
#include "player_shotgun_context.h"

#include "Game/Factory/projectile_factory.h"
#include "Utility/mi_math.h"

namespace
{
    constexpr float MIN_BULLET_SPEED = 25.0f;
    constexpr float MAX_BULLET_SPEED = 45.0f;
    constexpr float MIN_BULLET_RADIUS = 0.25f;
    constexpr float MAX_BULLET_RADIUS = 0.75f;
    constexpr float BULLET_LIFETIME = 3.0f;

    constexpr CollisionLayerMask SHOTGUN_HIT_LAYER_MASK =
        COLLISION_LAYER_MASK_ALL &
        ~CollisionLayerToMask(CollisionLayer::Player) &
        ~CollisionLayerToMask(CollisionLayer::Bullet);
}

void PlayerShotgunFiring::Fire(PlayerShotgunContext& context, const FireRequest& request)
{
    if (!context.scene) return;

    const float chargeRate = MiMath::Clamp(request.chargeRate, 0.0f, 1.0f);
    const float bulletSpeed = MiMath::Lerp(MIN_BULLET_SPEED, MAX_BULLET_SPEED, chargeRate);

    ProjectileFactory::BulletCreateDesc bulletDesc;
    bulletDesc.position = MiMath::Add(request.muzzlePosition, MiMath::Multiply(request.fireDirection, 0.5f)); // 少し前方に出す
    bulletDesc.velocity = MiMath::Multiply(
        MiMath::Normalize(request.fireDirection),
        bulletSpeed);
    bulletDesc.radius = MiMath::Lerp(MIN_BULLET_RADIUS, MAX_BULLET_RADIUS, chargeRate);
    bulletDesc.lifeTime = BULLET_LIFETIME;
    bulletDesc.layerMask = SHOTGUN_HIT_LAYER_MASK;

    ProjectileFactory::CreateBullet(context.scene, bulletDesc);

    context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::Fire);
}
