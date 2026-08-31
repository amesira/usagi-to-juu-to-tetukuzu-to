// player_shotgun_context.h
#pragma once

#include "player_shotgun_settings_asset.h"
#include "player_shotgun_aim.h"
#include "player_shotgun_charging.h"
#include "player_shotgun_firing.h"
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
    unsigned int gunLBoneIndex = static_cast<unsigned int>(-1);
};

struct PlayerShotgunContext 
{
    class PlayerShotgunAction* owner = nullptr;
    class IScene* scene = nullptr;

    PlayerShotgunRuntimeState runtimeState;
    PlayerShotgunReferences references;
    const PlayerShotgunSettingsAsset* settingsAsset = nullptr;
    const auto& settings() const {
        return settingsAsset->GetData();
    }

    class TransformComponent* playerTransform = nullptr;
    class ModelComponent* playerModel = nullptr;
    const struct PlayerRuntimeState* playerRuntimeState = nullptr;

    class TransformComponent* cameraTransform = nullptr;
    class CameraComponent* cameraComponent = nullptr;
    class CameraControlBehavior* cameraControlBehavior = nullptr;
    const class CameraSettingsAsset* defaultCameraSettingsAsset = nullptr;
    const class CameraSettingsAsset* shotgunCameraSettingsAsset = nullptr;

    class PlayerAnimationController* animationController = nullptr;
    class PlayerLocomotionController* locomotionController = nullptr;
    class PlayerWeaponController* weaponController = nullptr;

    PlayerShotgunAim aim;
    PlayerShotgunCharging charging;
    PlayerShotgunFiring firing;
    PlayerShotgunEffects effects;

};
