//===================================================
// File  ：_/E10_Locomotion/enemy_move.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_move.h"
#include <algorithm>
#include <cmath>
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"

#include "Engine/Core/game_object.h"

#include "Engine/Component/collider_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Component/transform_component.h"

#include "Engine/Processor/PhysicsPass/Collision/collision_query.h"
#include "Engine/Processor/PhysicsPass/Collision/collision_utility.h"

#include "Utility/mi_math.h"

void EnemyMove::Initialize(EnemyContext& context, const EnemyMoveSettingsAsset* settingsAsset)
{
    GameObject* owner = context.transform ? context.transform->GetOwner() : nullptr;
    if (owner == nullptr) return;

    m_context.owner = owner;
    m_context.scene = context.scene;
    m_context.transform = context.transform;
    m_context.rigidbody = context.rigidbody;
    m_context.collider = owner ? owner->GetComponent<CapsuleColliderComponent>() : nullptr;
    m_context.settingsAsset = settingsAsset;
    m_context.runtimeState = {};

    m_context.moveMotor = &m_motor;
    m_context.moveEffects = &m_effects;

    m_pathFollower.Initialize(m_context.settings().waypointReachDistance);
    m_effects.Initialize(m_context);
    context.pathFollower = &m_pathFollower;
}

void EnemyMove::Finalize()
{
    m_effects.Finalize();
    m_pathFollower.ClearPath();
    m_context = {};
}

void EnemyMove::UpdateMove(EnemyContext& context, const EnemyMoveIntent& intent, float deltaTime)
{
    if (!m_context.transform || !m_context.rigidbody) return;
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;
    
    m_context.runtimeState.isGrounded = CheckGrounded();
    m_context.runtimeState.desiredPosition = m_context.transform->GetPosition();

    // 強制移動は位置の基準を置き換える。通常移動・重力の加算は各フラグで制御する。
    if (intent.forceMoveIntent.isActive) {
        m_context.runtimeState.desiredPosition = intent.forceMoveIntent.targetPosition;
    }
    
    // === ジャンプ要求 ===
    /*if (input.triggerJumpCommand && m_context.runtimeState.m_isGrounded && moveIntent.canJump) {
        m_context.runtimeState.physicsVelocity.y = m_context.settings().jumpForce * moveIntent.jumpPowerMultiplier;
        m_context.runtimeState.isGrounded = false;

        m_context.moveEffects.PlayEffects(m_context, PlayerMoveEffects::EffectsType::Jump);
        m_requestJumpAnimation = true;
    }*/

    // === 移動処理 ===
    m_context.moveMotor->UpdateMotor(m_context, intent, deltaTime);

    // === 目標位置を算出し、速度を逆算 ===
    {
        // 速度で目標位置を更新
        ApplyControlVelocity(m_context.runtimeState.desiredPosition, deltaTime);
        ApplyPhysicsVelocity(m_context.runtimeState.desiredPosition, deltaTime);

        // 現在位置と目標位置の差分を計算してRigidbodyに反映
        XMFLOAT3 currentPosition = m_context.transform->GetPosition();
        XMFLOAT3 desiredPosition = m_context.runtimeState.desiredPosition;
        XMFLOAT3 deltaPosition = MiMath::Subtract(desiredPosition, currentPosition);

        // Rigidbodyの速度を設定
        XMFLOAT3 newVelocity = MiMath::Multiply(deltaPosition, 1.0f / deltaTime);
        m_context.rigidbody->SetVelocity(newVelocity);
    }

    // === 回転処理 ===
    if (intent.canRotate && MiMath::Length(intent.rotateDirection) > 0.01f) {
        XMFLOAT4 targetRot = MiMath::LookRotation(intent.rotateDirection, { 0.0f, 1.0f, 0.0f });
        context.transform->SetRotation(targetRot);
    }

    // === エフェクト処理 ===
    m_context.moveEffects->UpdateEffects(m_context, deltaTime);

    context.runtimeState.controlVelocity = m_context.runtimeState.controlVelocity;
    context.runtimeState.isGrounded = m_context.runtimeState.isGrounded;
}


#pragma region 速度計算
/// @brief 下向きのSphereCastで接地状態を判定する
bool EnemyMove::CheckGrounded()
{
    if (!m_context.transform || !m_context.collider || !m_context.scene) {
        return false;
    }

    // 上昇中は足元に地面が残っていても接地扱いにしない
    if (m_context.runtimeState.physicsVelocity.y > 0.0f) {
        return false;
    }

    const auto& settings = m_context.settings();
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

    return hasHit;
}

/// @brief 制御速度を適用する
void EnemyMove::ApplyControlVelocity(XMFLOAT3& outPosition, float deltaTime)
{
    outPosition = MiMath::Add(outPosition, MiMath::Multiply(m_context.runtimeState.controlVelocity, deltaTime));
}

/// @brief 物理速度を適用する
void EnemyMove::ApplyPhysicsVelocity(XMFLOAT3& outPosition, float deltaTime)
{
    outPosition = MiMath::Add(outPosition, MiMath::Multiply(m_context.runtimeState.physicsVelocity, deltaTime));
}

#pragma endregion
