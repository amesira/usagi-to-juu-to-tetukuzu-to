//===================================================
// player_move_behavior.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/03/25
//===================================================
#include "player_move_behavior.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include "Utility/mi_math.h"

#include "Engine/Editor/BaseEditor/inspector_view_window.h"
#include "Engine/engine_service_locator.h"

// === Component ===
#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/rigidbody_component.h"
#include "Engine/Framework/Component/camera_component.h"
#include "Engine/Framework/Component/animation_component.h"

// === Player ===
#include "Game/ActorBehavior/Player/player_behavior.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include <cmath>

namespace {
    #define DATA_LOADER EngineServiceLocator::GetAssetManager()->GetDataAssetLoader()
}

void PlayerMoveBehavior::Start() {}

void PlayerMoveBehavior::Update() {}

void PlayerMoveBehavior::DrawComponentInspector()
{
    if (InspectorViewWindow::BeginComponentSection(this, "Player Move Behavior")) 
    {
        ImGui::Text("=== Runtime State ===");
        ImGui::Text("Control Velocity: (%.2f, %.2f, %.2f)", m_context.runtimeState.m_controlVelocity.x, m_context.runtimeState.m_controlVelocity.y, m_context.runtimeState.m_controlVelocity.z);
        ImGui::Text("Physics Velocity: (%.2f, %.2f, %.2f)", m_context.runtimeState.m_physicsVelocity.x, m_context.runtimeState.m_physicsVelocity.y, m_context.runtimeState.m_physicsVelocity.z);
        ImGui::Text("Desired Position: (%.2f, %.2f, %.2f)", m_context.runtimeState.m_desiredPosition.x, m_context.runtimeState.m_desiredPosition.y, m_context.runtimeState.m_desiredPosition.z);
    }

    InspectorViewWindow::EndComponentSection();
}

// -----------------------------------------------

/// @brief PlayerMoveBehaviorのコンテキストを設定する
void PlayerMoveBehavior::SetupContext(const PlayerContext& playerContext)
{
    m_context.owner = this;

    m_context.transform = playerContext.transform;
    m_context.rigidbody = playerContext.owner->GetOwner()->GetComponent<RigidbodyComponent>();

    m_context.moveMotor = {};
    m_context.moveRotate = {};
    m_context.moveEffects = {};

    if (m_context.settingsAsset == nullptr) {
        m_context.settingsAsset = DATA_LOADER->GetAsset<PlayerMoveSettingsAsset>("asset/Data/player_move_settings.data.json", true);
    }

    // 仮
    m_animationComponent = playerContext.owner->GetOwner()->GetComponent<AnimationComponent>();
}

void PlayerMoveBehavior::UpdateMove(const PlayerContext& context, const PlayerInput& input, const PlayerMoveIntent& moveIntent, float deltaTime)
{
    m_context.runtimeState.m_isGrounded = m_context.rigidbody->GetIsGrounded();

    // 移動アクション類の更新要求などはここ？（ジャンプやブリンクなど）
    if (input.triggerJumpCommand) {
        // ジャンプ処理の要求をここで行う
        m_context.runtimeState.m_physicsVelocity.y = m_context.settings().jumpForce; // ジャンプ力を設定
        m_context.runtimeState.m_isGrounded = false; // ジャンプ中は地面に接地していない状態にする
    }

    // 現在位置の取得
    m_context.runtimeState.m_desiredPosition = m_context.transform->GetPosition();

    // Blinkや攻撃による進みなどMoveWithCollision的な移動はここ（位置の上書き的な挙動に近い移動処理）

    // 移動処理の更新
    m_context.moveMotor.UpdateMove_Motor(m_context, moveIntent, deltaTime);

    // 速度で目標位置を更新
    ApplyControlVelocity(m_context.runtimeState.m_desiredPosition, deltaTime);
    ApplyPhysicsVelocity(m_context.runtimeState.m_desiredPosition, deltaTime);

    // 現在位置と目標位置の差分を計算してRigidbodyに反映
    XMFLOAT3 currentPosition = m_context.transform->GetPosition();
    XMFLOAT3 desiredPosition = m_context.runtimeState.m_desiredPosition;
    XMFLOAT3 deltaPosition = MiMath::Subtract(desiredPosition, currentPosition);

    // Rigidbodyの速度を設定
    XMFLOAT3 newVelocity = MiMath::Multiply(deltaPosition, 1.0f / deltaTime);
    m_context.rigidbody->SetVelocity(newVelocity);

    // 回転処理
    if (moveIntent.canRotate && MiMath::Length(moveIntent.rotateDirection) > 0.01f) {
        m_context.moveRotate.UpdateMove_Rotate(m_context, moveIntent, deltaTime);
    }

    // === 仮：アニメーション更新 ===
    if (m_animationComponent) {
        if (MiMath::Length(m_context.runtimeState.m_controlVelocity) > 0.01f) {
            m_animationComponent->SetAnimationState(1, 2.0f); // Moveアニメーションを再生
        }
        else {
            m_animationComponent->SetAnimationState(0, 1.0f); // Idleアニメーションを再生
        }
    }
}

/// @brief 制御速度を適用する
void PlayerMoveBehavior::ApplyControlVelocity(XMFLOAT3& outPosition, float deltaTime)
{
    outPosition = MiMath::Add(outPosition, MiMath::Multiply(m_context.runtimeState.m_controlVelocity, deltaTime));
}

/// @brief 物理速度を適用する
void PlayerMoveBehavior::ApplyPhysicsVelocity(XMFLOAT3& outPosition, float deltaTime)
{
    outPosition = MiMath::Add(outPosition, MiMath::Multiply(m_context.runtimeState.m_physicsVelocity, deltaTime));
}


//
//// 移動更新処理
//void PlayerMoveBehavior::UpdateMove(const PlayerContext& context, const PlayerInput& input, const PlayerMoveRequest& moveRequest, float deltaTime)
//{
//    if (!m_rigidbody) return;
//
//    XMFLOAT3 velocity = m_rigidbody->GetVelocity();
//
//    // 移動できない場合は速度を0にして終了
//    if (!moveRequest.canMove) {
//        velocity.x = 0.0f;
//        velocity.z = 0.0f;
//        m_rigidbody->SetVelocity(velocity);
//        return;
//    }
//
//    XMFLOAT3 moveDirection = { input.moveDirection.x, 0.0f, input.moveDirection.z };
//    moveDirection = MiMath::Multiply(MiMath::Normalize(moveDirection), m_moveSpeed);
//
//    velocity.x += moveDirection.x * deltaTime * m_acceleration * moveRequest.speedMultiplier;
//    velocity.z += moveDirection.z * deltaTime * m_acceleration * moveRequest.speedMultiplier;
//
//    velocity.x = MiMath::Clamp(velocity.x, -m_moveSpeed * moveRequest.speedMultiplier, m_moveSpeed * moveRequest.speedMultiplier);
//    velocity.z = MiMath::Clamp(velocity.z, -m_moveSpeed * moveRequest.speedMultiplier, m_moveSpeed * moveRequest.speedMultiplier);
//
//    // ジャンプ処理
//    if (input.triggerJumpCommand && m_rigidbody->GetIsGrounded()) {
//        velocity.y = m_jumpForce;
//    }
//
//    m_rigidbody->SetVelocity(velocity);
//}
//
//// 回転更新処理
//void PlayerMoveBehavior::UpdateRotation(const PlayerContext& context, const PlayerInput& input, const PlayerMoveRequest& moveRequest, float deltaTime)
//{
//    if (!moveRequest.canRotate) return;
//    if (moveRequest.rotationMode == PlayerRotationMode::Locked) return;
//    if (!m_transform || !m_mainCamera) return;
//
//    // カメラ正面を取得
//    XMFLOAT3 forward = m_mainCamera->GetForward();
//
//    switch (moveRequest.rotationMode) {
//    case PlayerRotationMode::CameraForward: // === 常にカメラ正面を向く ===
//        if (input.horizontal > 0.01f) {
//            m_spriteRenderer->SetFlipX(true);
//        }
//        else if (input.horizontal < -0.01f) {
//            m_spriteRenderer->SetFlipX(false);
//        }
//        break;
//    case PlayerRotationMode::AimForward: // === エイム状態 ===
//        forward = MiMath::Add(forward, MiMath::Multiply(m_mainCamera->GetRight(), -1.0f));
//        forward = MiMath::Normalize(forward);
//        m_spriteRenderer->SetFlipX(true);
//        break;
//    default:
//        break;
//    }
//
//    // 正面方向からY軸回転の角度を計算
//    const float billboardAngleY = atan2f(forward.x, forward.z);
//    XMFLOAT4 targetRotation = MiMath::QuaternionFromEuler({ 0.0f, billboardAngleY, 0.0f });
//    XMFLOAT4 currentRotation = m_transform->GetRotation();
//    currentRotation = MiMath::Slerp(currentRotation, targetRotation, m_rotationSpeed * deltaTime);
//
//    m_transform->SetRotation(currentRotation);
//    m_currentAngleY = m_transform->GetEulerAngle().y;
//}
