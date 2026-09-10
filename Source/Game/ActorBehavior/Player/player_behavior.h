//---------------------------------------------------
// File  ：_/ActorBehavior/Player/player_behavior.h
// Date  ：2026/07/30
// Author：Miu Kitamura
// 
// ・プレイヤーの挙動を統括する司令塔的なBehaviorComponent
//---------------------------------------------------
#ifndef PLAYER_BEHAVIOR_H
#define PLAYER_BEHAVIOR_H

#include "Engine/Component/behavior_component.h"
#include "Game/ActorBehavior/Player/player_animation_controller.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_behavior.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_settings_asset.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"

#include "Game/ActorBehavior/Player/P20_Condition/player_condition_machine.h"
#include "Game/ActorBehavior/Player/P30_Action/player_action_machine.h"
#include "Game/ActorBehavior/Player/P30_Action/Attack/DualPistols/player_dual_pistols_action.h"
#include "Game/ActorBehavior/Player/P30_Action/Attack/Shotgun/player_shotgun_action.h"
#include "Game/ActorBehavior/Player/P40_Weapon/player_weapon_controller.h"

class TransformComponent;
class CameraComponent;

/// @brief プレイヤーの挙動を統括する司令塔的なBehaviorComponent
class PlayerBehavior : public BehaviorComponent {
private:
    PlayerContext m_context;
    PlayerInput m_input;

    bool m_isInputEnabled = true;
    bool m_registeredEntityToMetaAI = false;

    // === プレイヤー構成要素の実体 ===
    PlayerLocomotionController m_locomotionController;
    PlayerMoveBehavior m_moveBehavior;
    PlayerConditionMachine m_conditionMachine;
    PlayerActionMachine m_actionMachine;
    PlayerWeaponController m_weaponController;

    PlayerAnimationController m_animationController;

    PlayerMoveSettingsAsset* m_moveSettings = nullptr;

    // === プレイヤーアクションの実体 ===
    PlayerDualPistolsAction m_dualPistolsAction;
    PlayerShotgunAction m_shotgunAction;

    PlayerDualPistolsSettingsAsset* m_dualPistolsSettings = nullptr;
    PlayerShotgunSettingsAsset* m_shotgunSettings = nullptr;

    const class CameraSettingsAsset* m_shotgunCameraSettings = nullptr;

public:
    ~PlayerBehavior() = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    void SetupPlayerMove(PlayerMoveSettingsAsset* settings) {
        m_moveSettings = settings;
    }

    void SetupPlayerShotgun(
        PlayerShotgunSettingsAsset* settings,
        const CameraSettingsAsset* cameraSettings) {
        m_shotgunSettings = settings;
        m_shotgunCameraSettings = cameraSettings;
    }

    void SetupPlayerDualPistols(
        PlayerDualPistolsSettingsAsset* settings) {
        m_dualPistolsSettings = settings;
    }

private:
    /// @brief プレイヤーの入力を更新する
    PlayerInput UpdateInput();

};

#endif
