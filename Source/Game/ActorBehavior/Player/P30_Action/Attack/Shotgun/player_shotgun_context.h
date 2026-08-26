// player_shotgun_context.h
#pragma once

#include "player_shotgun_settings_asset.h"
#include "player_shotgun_aim.h"
#include "player_shotgun_charging.h"
#include "player_shotgun_effects.h"

struct PlayerShotgunRuntimeState 
{
    enum class Phase {
        None,
        Entering,
        Aiming,
        Charging,
        Firing,
        Recovery,
        Exiting,
    };

    Phase phase = Phase::None;
    float phaseTimer = 0.0f;
};

struct PlayerShotgunReferences 
{
    class TransformComponent* muzzleTransform = nullptr;

    class ParticleSystemComponent* chargeParticle = nullptr;
    class ParticleSystemComponent* muzzleFlashParticle = nullptr;
    class LightComponent* chargeLight = nullptr;
};

struct PlayerShotgunContext 
{
    class PlayerShotgunAction* owner = nullptr;

    PlayerShotgunRuntimeState runtimeState;
    PlayerShotgunReferences references;
    const PlayerShotgunSettingsAsset* settingsAsset = nullptr;
    const auto& settings() {
        return settingsAsset->GetData();
    }

    class TransformComponent* playerTransform = nullptr;

    class TransformComponent* cameraTransform = nullptr;
    class CameraComponent* cameraComponent = nullptr;
    class CameraControlBehavior* cameraControlBehavior = nullptr;

    class PlayerLocomotionController* locomotionController = nullptr;
    class PlayerWeaponController* weaponController = nullptr;

    PlayerShotgunAim aim;
    PlayerShotgunCharging charging;
    PlayerShotgunEffects effects;

};