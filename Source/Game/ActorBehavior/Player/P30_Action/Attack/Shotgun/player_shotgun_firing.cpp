#include "Game/ControllerBehavior/Audio/game_audio_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
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

#include "Game/ActorBehavior/Player/P40_Weapon/player_weapon_controller.h"

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

    float chargeRate = MiMath::Clamp(request.chargeRate, 0.0f, 1.0f);
    if (context.weaponController) {
        if (!context.weaponController->CanConsume(1)) {
            return; // 弾薬が足りない場合は発射しない
        }
        float consumeCost = context.weaponController->TryConsume(chargeRate * 25);

        chargeRate = consumeCost / 25.0f; // 実際に消費できた量に応じてチャージ率を調整
    }

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
    bulletDesc.damage = settings.bulletDamage * (1.0f + chargeRate * (settings.chargeDamageMultiplier - 1.0f));

    ProjectileFactory::CreateBullet(context.scene, bulletDesc);

    context.effects.PlayEffects(context, PlayerShotgunEffects::EffectsType::Fire);
}
