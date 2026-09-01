//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_context.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃アクションで共有するコンテキスト
//---------------------------------------------------
#pragma once

#include "Game/ActorBehavior/Player/P30_Action/player_muzzle_state.h"
#include "player_dual_pistols_settings_asset.h"
#include "player_dual_pistols_aim.h"
#include "player_dual_pistols_firing.h"
#include "player_dual_pistols_effects.h"
#include "player_dual_pistols_rapid_fire.h"
#include "player_dual_pistols_slash_burst.h"

struct PlayerDualPistolsRuntimeState {
    enum class Phase {
        None,
        Entering,
        SlashBurst,
        RapidFire,
        Exiting,
    };

    Phase phase = Phase::None;
    float phaseTimer = 0.0f;

    bool releaseAttackInput = false;

    int comboStep = 0;

    PlayerMuzzleState leftMuzzle;
    PlayerMuzzleState rightMuzzle;
};

struct PlayerDualPistolsReferences {
    unsigned int gunLBoneIndex = static_cast<unsigned int>(-1);
    unsigned int gunRBoneIndex = static_cast<unsigned int>(-1);
};

struct PlayerDualPistolsContext {
    class PlayerDualPistolsAction* owner = nullptr;
    class IScene* scene = nullptr;

    PlayerDualPistolsRuntimeState runtimeState;
    PlayerDualPistolsReferences references;

    const PlayerDualPistolsSettingsAsset* settingsAsset = nullptr;
    const auto& settings() const {
        return settingsAsset->GetData();
    }

    class TransformComponent* playerTransform = nullptr;
    class ModelComponent* playerModel = nullptr;
    const struct PlayerRuntimeState* playerRuntimeState = nullptr;

    class TransformComponent* cameraTransform = nullptr;
    class CameraComponent* cameraComponent = nullptr;
    class CameraControlBehavior* cameraControlBehavior = nullptr;

    class PlayerAnimationController* animationController = nullptr;
    class PlayerLocomotionController* locomotionController = nullptr;
    class PlayerWeaponController* weaponController = nullptr;

    PlayerDualPistolsAim aim;
    PlayerDualPistolsFiring firing;
    PlayerDualPistolsEffects effects;
    PlayerDualPistolsRapidFire rapidFire;
    PlayerDualPistolsSlashBurst slashBurst;
};
