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
#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/camera_component.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_utility.h"

// === Player ===
#include "Game/ActorBehavior/Player/player_behavior.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include <cmath>

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
        ImGui::Text("Grounded: %s", m_context.runtimeState.m_isGrounded ? "true" : "false");
    }

    InspectorViewWindow::EndComponentSection();
}

// -----------------------------------------------

/// @brief PlayerMoveBehaviorのコンテキストを設定する
void PlayerMoveBehavior::Initialize(const PlayerContext& playerContext, PlayerMoveReferences& references, PlayerMoveSettingsAsset* settings)
{
    m_context.owner = this;

    GameObject* player = playerContext.owner->GetOwner();
    m_context.transform = playerContext.transform;
    m_context.rigidbody = player->GetComponent<RigidbodyComponent>();
    m_context.collider = player->GetComponent<BoxColliderComponent>();

    m_context.moveMotor = {};
    m_context.moveRotate = {};
    m_context.moveEffects = {};

    m_context.runtimeState = {};
    m_context.references = references;
    m_context.settingsAsset = settings;

}

void PlayerMoveBehavior::UpdateMove(PlayerContext& context, const PlayerInput& input, const PlayerMoveIntent& moveIntent, float deltaTime)
{
    m_context.runtimeState.m_isGrounded = CheckGrounded();

    // ジャンプ要求
    if (input.triggerJumpCommand && m_context.runtimeState.m_isGrounded && moveIntent.canJump) {
        m_context.runtimeState.m_physicsVelocity.y = m_context.settings().jumpForce * moveIntent.jumpPowerMultiplier;
        m_context.runtimeState.m_isGrounded = false;
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::Jump);
    }

    // 現在位置の取得
    m_context.runtimeState.m_desiredPosition = m_context.transform->GetPosition();

    // ブリンク要求（仮）
    if (input.triggerDashCommand) {
        // ブリンク処理の要求をここで行う
        XMFLOAT3 blinkDirection = moveIntent.moveDirection;
        float blinkDistance = 5.0f; // ブリンク距離（仮）
        XMFLOAT3 blinkOffset = MiMath::Multiply(blinkDirection, blinkDistance);
        m_context.runtimeState.m_desiredPosition = MiMath::Add(m_context.runtimeState.m_desiredPosition, blinkOffset);
    }

    // Blinkや攻撃による進みなどMoveWithCollision的な移動はここ（位置の上書き的な挙動に近い移動処理）

    // === 移動処理 ===
    m_context.moveMotor.UpdateMotor(m_context, moveIntent, deltaTime);

    // === 目標位置を算出し、速度を逆算 ===
    {
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
    }

    // === 回転処理 ===
    if (moveIntent.canRotate && MiMath::Length(moveIntent.rotateDirection) > 0.01f) {
        m_context.moveRotate.UpdateRotate(m_context, moveIntent, deltaTime);
    }

    // === エフェクト処理 ===
    m_context.moveEffects.UpdateEffects(m_context, deltaTime);

    // === アニメーション要求 ===
    if (!m_context.runtimeState.m_isGrounded) {
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::Falling);
    }
    else {
        context.animationController->PlayAnimation(
            MiMath::Length(m_context.runtimeState.m_controlVelocity) > 0.01f
                ? PlayerAnimationController::Animation::Running
                : PlayerAnimationController::Animation::Idle);
    }
}

/// @brief 下向きのSphereCastで接地状態を判定する
bool PlayerMoveBehavior::CheckGrounded()
{
    if (!m_context.transform || !m_context.collider || !GetOwner()->GetScene()) {
        return false;
    }

    // 上昇中は足元に地面が残っていても接地扱いにしない
    if (m_context.runtimeState.m_physicsVelocity.y > 0.0f) {
        return false;
    }

    const PlayerMoveSettings::Data& settings = m_context.settings();
    const XMFLOAT3 colliderScale = m_context.collider->GetScale();
    const XMFLOAT3 colliderCenter = m_context.collider->GetCenter();
    const XMFLOAT4 playerRotation = m_context.transform->GetRotation();

    // Playerのローカル座標における足元を求める
    const XMFLOAT3 localFootPosition = {
        colliderCenter.x,
        colliderCenter.y - colliderScale.y * 0.5f,
        colliderCenter.z
    };
    const XMFLOAT3 footPosition = MiMath::Add(
        m_context.transform->GetPosition(),
        MiMath::RotateVector(playerRotation, localFootPosition));

    // SphereCastのoriginは球の中心なので、足元から半径分だけ上に置く
    const XMFLOAT3 castOrigin = {
        footPosition.x,
        footPosition.y + settings.groundCheckRadius,
        footPosition.z
    };

    RaycastHit hit;
    const bool hasHit = CollisionQuery::SphereCast(
        GetOwner()->GetScene(),
        /*out*/ hit,
        castOrigin,
        { 0.0f, -1.0f, 0.0f },
        settings.groundCheckRadius,
        settings.groundCheckDistance,
        CollisionLayerToMask(CollisionLayer::Field));

    return hasHit;
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
