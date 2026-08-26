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

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_settings_asset.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"

#include "Game/ActorBehavior/Player/P20_Condition/player_condition_machine.h"
#include "Game/ActorBehavior/Player/P30_Action/player_action_machine.h"

class TransformComponent;
class CameraComponent;

/// @brief プレイヤーの挙動を統括する司令塔的なBehaviorComponent
class PlayerBehavior : public BehaviorComponent {
private:
    PlayerContext m_context;
    PlayerInput m_input;

    PlayerMoveReferences m_moveReferences;
    PlayerMoveSettingsAsset* m_moveSettings;

    // === プレイヤー構成要素の実体 ===
    PlayerLocomotionController m_locomotionController;
    PlayerConditionMachine m_conditionMachine;
    PlayerActionMachine m_actionMachine;

public:
    ~PlayerBehavior() = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    void SetupPlayerMove(PlayerMoveReferences& references, PlayerMoveSettingsAsset* settings) {
        m_moveReferences = references;
        m_moveSettings = settings;
    }

private:
    /// @brief プレイヤーの入力を更新する
    PlayerInput UpdateInput();

};

#endif
