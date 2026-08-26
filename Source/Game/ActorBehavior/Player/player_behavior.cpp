//---------------------------------------------------
// File  ：_/ActorBehavior/Player/player_behavior.cpp
// Date  ：2026/07/30
// Author：Miu Kitamura
// 
// ・プレイヤーの挙動を統括する司令塔的なBehaviorComponent
//---------------------------------------------------
#include "player_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/keyboard.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Device/mouse.h"
#include "Engine/Editor/BaseEditor/inspector_view_window.h"

#include "Engine/Component/camera_component.h"
#include "Engine/Component/transform_component.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_behavior.h"
#include "Game/ActorBehavior/Player/P20_Condition/player_condition_machine.h"
#include "Game/ActorBehavior/Player/P30_Action/player_action_machine.h"

#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"
#include "Game/PresBehavior/Camera/camera_control_behavior.h"

#include "Utility/mi_math.h"

using namespace DirectX;

void PlayerBehavior::Start()
{
    GameObject* owner = GetOwner();
    if (!owner) return;

    m_context.owner = this;
    m_context.transform = owner->GetComponent<TransformComponent>();
    m_context.locomotionController = &m_locomotionController;
    m_context.moveBehavior = owner->GetComponent<PlayerMoveBehavior>();

    m_context.conditionMachine = &m_conditionMachine;
    m_context.actionMachine = &m_actionMachine;
    m_context.weaponController = &m_weaponController;

    IScene* scene = owner->GetScene();
    if (!scene) return;
    GameObject* mainCamera = scene->GetGameObjectByName("MainCamera");
    if (mainCamera) {
        m_context.mainCameraTransform = mainCamera->GetComponent<TransformComponent>();
        m_context.mainCamera = mainCamera->GetComponent<CameraComponent>();

        CameraControlBehavior* cameraControl = mainCamera->GetComponent<CameraControlBehavior>();
        m_context.cameraControlBehavior = cameraControl;
    }

    // 構成要素の初期化
    m_locomotionController.Initialize();
    m_context.moveBehavior->Initialize(m_context, m_moveReferences, m_moveSettings);
    m_context.actionMachine->Initialize(m_context, m_input);

    // Shotgun用の参照が未設定の場合は、暫定的にPlayerのTransformを銃口として扱う
    if (!m_shotgunReferences.muzzleTransform) {
        m_shotgunReferences.muzzleTransform = m_context.transform;
    }

    // SettingsAssetがPrefabから渡されるまではデフォルト設定を使用する
    static PlayerShotgunSettingsAsset defaultShotgunSettings;
    if (!m_shotgunSettings) {
        m_shotgunSettings = &defaultShotgunSettings;
    }

    m_shotgunAction.Initialize(m_context, m_shotgunReferences, m_shotgunSettings);

    m_context.actionMachine->RegisterAction(m_dualPistolsAction);
    m_context.actionMachine->RegisterAction(m_shotgunAction);

}

void PlayerBehavior::Update()
{
    const float deltaTime = FPS_GetDeltaTime();
    m_input = UpdateInput();

    m_context.conditionMachine->Update(m_context, m_input);
    m_weaponController.Update(m_context, m_input);
    m_context.actionMachine->Update(m_context, m_input);

    // プレイヤーの移動挙動を更新する
    PlayerMoveIntent intent = m_context.locomotionController->BuildIntent(m_context, m_input);
    m_context.moveBehavior->UpdateMove(m_context, m_input, intent, deltaTime);
}

void PlayerBehavior::DrawComponentInspector()
{
    if (InspectorViewWindow::BeginComponentSection(this, "Player Behavior")) {
    }
    InspectorViewWindow::EndComponentSection();
}

/// @brief プレイヤーの入力を更新する
PlayerInput PlayerBehavior::UpdateInput()
{
    PlayerInput input = {};

    if (Keyboard_IsKeyDown(KK_D)) {
        input.horizontal = 1.0f;
    }
    else if (Keyboard_IsKeyDown(KK_A)) {
        input.horizontal = -1.0f;
    }

    if (Keyboard_IsKeyDown(KK_W)) {
        input.vertical = 1.0f;
    }
    else if (Keyboard_IsKeyDown(KK_S)) {
        input.vertical = -1.0f;
    }

    input.moveInput = { input.horizontal, 0.0f, input.vertical };

    if (m_context.mainCamera) {
        input.moveDirection = MiMath::Normalize(
            MiMath::Add(
                MiMath::Multiply(m_context.mainCamera->GetRight(), input.horizontal),
                MiMath::Multiply(m_context.mainCamera->GetForward(), input.vertical)));
    }

    input.triggerJumpCommand = Keyboard_IsKeyDownTrigger(KK_SPACE);
    input.triggerDashCommand = Keyboard_IsKeyDownTrigger(KK_LEFTSHIFT);

    input.triggerAimCommand = Mouse_IsButtonDownTrigger(Mouse_Button::RIGHT);
    input.holdAimCommand = Mouse_IsButtonDown(Mouse_Button::RIGHT);
    input.releaseAimCommand = Mouse_IsButtonUpTrigger(Mouse_Button::RIGHT);

    input.triggerAttackCommand = Mouse_IsButtonDownTrigger(Mouse_Button::LEFT);
    input.holdAttackCommand = Mouse_IsButtonDown(Mouse_Button::LEFT);
    input.releaseAttackCommand = Mouse_IsButtonUpTrigger(Mouse_Button::LEFT);

    return input;
}
