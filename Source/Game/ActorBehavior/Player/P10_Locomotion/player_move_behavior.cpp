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

#include "Engine/engine_service_locator.h"
#include "External/ImGui/imgui.h"

// === Component ===
#include "Engine/Component/transform_component.h"
#include "Game/ControllerBehavior/StageBounds/stage_bounds_controller_behavior.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/collider_component.h"
#include "Engine/Component/camera_component.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_utility.h"

// === Player ===
#include "Game/ActorBehavior/Player/player_behavior.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include "Game/PresBehavior/UI/Player/player_ui_behavior.h"

#include <cmath>

void PlayerMoveBehavior::DrawInspector()
{
    ImGui::Text("=== Move Runtime State ===");
    ImGui::Text("Control Velocity: (%.2f, %.2f, %.2f)", m_context.runtimeState.m_controlVelocity.x, m_context.runtimeState.m_controlVelocity.y, m_context.runtimeState.m_controlVelocity.z);
    ImGui::Text("Physics Velocity: (%.2f, %.2f, %.2f)", m_context.runtimeState.m_physicsVelocity.x, m_context.runtimeState.m_physicsVelocity.y, m_context.runtimeState.m_physicsVelocity.z);
    ImGui::Text("Desired Position: (%.2f, %.2f, %.2f)", m_context.runtimeState.m_desiredPosition.x, m_context.runtimeState.m_desiredPosition.y, m_context.runtimeState.m_desiredPosition.z);
    ImGui::Text("Grounded: %s", m_context.runtimeState.m_isGrounded ? "true" : "false");
}

// -----------------------------------------------

/// @brief PlayerMoveBehaviorのコンテキストを設定する
void PlayerMoveBehavior::Initialize(const PlayerContext& playerContext, PlayerMoveSettingsAsset* settings)
{
    GameObject* player = playerContext.owner->GetOwner();
    m_context.owner = player;
    m_context.scene = playerContext.scene;

    m_context.transform = playerContext.transform;
    m_context.rigidbody = player->GetComponent<RigidbodyComponent>();
    m_context.collider = player->GetComponent<CapsuleColliderComponent>();

    m_context.animationController = playerContext.animationController;
    m_context.uiBehavior = playerContext.uiBehavior;

    m_context.moveMotor = {};
    m_context.moveRotate = {};
    m_context.moveEffects = {};

    m_context.runtimeState = {};
    m_context.settingsAsset = settings;
    m_context.moveEffects.Initialize(m_context);

}

void PlayerMoveBehavior::UpdateMove(PlayerContext& context, const PlayerInput& input, const PlayerMoveIntent& moveIntent, float deltaTime)
{
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    if (moveIntent.pauseMovement) {
        // 内部の物理速度は保持し、Rigidbodyへの移動出力だけ止める。
        m_context.rigidbody->SetVelocity({ 0.0f, 0.0f, 0.0f });
        return;
    }

    m_context.runtimeState.m_isGrounded = CheckGrounded();
    m_context.runtimeState.m_desiredPosition = m_context.transform->GetPosition();
    if (moveIntent.forceMoveIntent.isActive) {
        m_context.runtimeState.m_desiredPosition = moveIntent.forceMoveIntent.targetPosition;
    }

    // === ジャンプ要求 ===
    if (input.triggerJumpCommand && m_context.runtimeState.m_isGrounded && moveIntent.canJump) {
        m_context.runtimeState.m_physicsVelocity.y = m_context.settings().jumpForce * moveIntent.jumpPowerMultiplier;
        m_context.runtimeState.m_isGrounded = false;

        m_context.moveEffects.PlayEffects(m_context, PlayerMoveEffects::EffectsType::Jump);
        m_requestJumpAnimation = true;
    }

    // === 移動処理 ===
    m_context.moveMotor.UpdateMotor(m_context, moveIntent, deltaTime);

    // === 目標位置を算出し、速度を逆算 ===
    {
        // 速度で目標位置を更新
        ApplyControlVelocity(m_context.runtimeState.m_desiredPosition, deltaTime);
        ApplyPhysicsVelocity(m_context.runtimeState.m_desiredPosition, deltaTime);
        if (auto* bounds = StageBoundsControllerBehavior::Find(m_context.transform)) {
            const auto result = bounds->Resolve(m_context.transform, m_context.runtimeState.m_desiredPosition);
            m_context.runtimeState.m_desiredPosition = result.position;
            result.ClipVelocity(m_context.runtimeState.m_controlVelocity);
            result.ClipVelocity(m_context.runtimeState.m_physicsVelocity);
            m_context.moveMotor.ApplyBounds(result);
            if (result.floor && m_context.runtimeState.m_physicsVelocity.y <= 0) m_context.runtimeState.m_isGrounded = true;
        }

        // 現在位置と目標位置の差分を計算してRigidbodyに反映
        XMFLOAT3 currentPosition = m_context.transform->GetPosition();
        XMFLOAT3 desiredPosition = m_context.runtimeState.m_desiredPosition;
        XMFLOAT3 deltaPosition = MiMath::Subtract(desiredPosition, currentPosition);

        // Rigidbodyの速度を設定
        XMFLOAT3 newVelocity = MiMath::Multiply(deltaPosition, 1.0f / deltaTime);
        if (auto* bounds = StageBoundsControllerBehavior::Find(m_context.transform)) {
            newVelocity = bounds->ConstrainVelocity(m_context.transform, m_context.rigidbody, newVelocity, deltaTime);
        }
        m_context.rigidbody->SetVelocity(newVelocity);
    }

    // === 回転処理 ===
    m_context.moveRotate.UpdateRotate(m_context, moveIntent, deltaTime);

    // === エフェクト処理 ===
    m_context.moveEffects.UpdateEffects(m_context, deltaTime);

    // 他のプレイヤー機能から参照する汎用ランタイム状態を更新する。
    context.runtimeState.m_controlVelocity = m_context.runtimeState.m_controlVelocity;
    context.runtimeState.m_physicsVelocity = m_context.runtimeState.m_physicsVelocity;
    context.runtimeState.m_desiredPosition = m_context.runtimeState.m_desiredPosition;
    context.runtimeState.m_isGrounded = m_context.runtimeState.m_isGrounded;

    XMFLOAT3 playerForward = m_context.transform->GetForward();
    playerForward.y = 0.0f;
    playerForward = MiMath::Normalize(playerForward);

    XMFLOAT3 playerRight = m_context.transform->GetRight();
    playerRight.y = 0.0f;
    playerRight = MiMath::Normalize(playerRight);

    context.runtimeState.localMoveParameter = {
        MiMath::Dot(moveIntent.moveDirection, playerRight) * moveIntent.moveInputMagnitude,
        MiMath::Dot(moveIntent.moveDirection, playerForward) * moveIntent.moveInputMagnitude,
    };
    context.runtimeState.cameraBaseMoveParameter = {
        MiMath::Dot(moveIntent.moveDirection, context.mainCamera->GetRight()) * moveIntent.moveInputMagnitude,
        MiMath::Dot(moveIntent.moveDirection, context.mainCamera->GetForward()) * moveIntent.moveInputMagnitude,
    };

    UpdateAnimation(context, moveIntent, deltaTime);
    UpdateUi(context, moveIntent, deltaTime);
}

#pragma region 速度計算
/// @brief 下向きのSphereCastで接地状態を判定する
bool PlayerMoveBehavior::CheckGrounded()
{
    if (!m_context.transform || !m_context.collider || !m_context.scene) {
        return false;
    }

    // 上昇中は足元に地面が残っていても接地扱いにしない
    if (m_context.runtimeState.m_physicsVelocity.y > 0.0f) {
        return false;
    }

    const PlayerMoveSettings::Data& settings = m_context.settings();
    const XMFLOAT3 colliderScale = {
        m_context.collider->GetRadius() * 2.0f,
        m_context.collider->GetHeight(),
        m_context.collider->GetRadius() * 2.0f
    };
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
        m_context.scene,
        /*out*/ hit,
        castOrigin,
        { 0.0f, -1.0f, 0.0f },
        settings.groundCheckRadius,
        settings.groundCheckDistance,
        CollisionLayerToMask(CollisionLayer::Field));

    auto* bounds = StageBoundsControllerBehavior::Find(m_context.transform);
    return hasHit || (bounds && bounds->IsGrounded(m_context.transform, settings.groundCheckDistance));
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
#pragma endregion

void PlayerMoveBehavior::UpdateAnimation(PlayerContext& context, const PlayerMoveIntent& moveIntent, float deltaTime)
{
    if (m_requestJumpAnimation) {
        context.animationController->PlayAnimation(PlayerAnimationController::Animation::Jump);
        m_requestJumpAnimation = false;
    }

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

void PlayerMoveBehavior::UpdateUi(PlayerContext& context, const PlayerMoveIntent& moveIntent, float deltaTime)
{
    if (m_context.uiBehavior) {
        // プレイヤーの移動によって、消失点を少しずらす
        XMFLOAT2 targetOffset = { 0.0f, 0.0f };
        if (moveIntent.moveInputMagnitude > 0.01f) {
            targetOffset.x = context.runtimeState.cameraBaseMoveParameter.x
                * m_context.settings().uiVanishingPointOffset.x;
        }

        if (!m_context.runtimeState.m_isGrounded) {
            if (m_context.runtimeState.m_physicsVelocity.y > 0.01f) {
                targetOffset.y = -m_context.settings().uiVanishingPointOffset.y;
            }
            else if (m_context.runtimeState.m_physicsVelocity.y < -0.01f) {
                targetOffset.y = m_context.settings().uiVanishingPointOffset.y;
            }
        }

        // オフセット値をスムーズに補間する
        m_vanishOffset.x = MiMath::SmoothDamp(m_vanishOffset.x, targetOffset.x,
            m_vanishOffsetVelocity.x, m_context.settings().uiVanishingPointSmoothTime.x, deltaTime);
        m_vanishOffset.y = MiMath::SmoothDamp(m_vanishOffset.y, targetOffset.y,
            m_vanishOffsetVelocity.y, m_context.settings().uiVanishingPointSmoothTime.y, deltaTime);

        m_context.uiBehavior->SetPerspectiveVanishingPointOffset(m_vanishOffset);
    }
}
