//---------------------------------------------------
// File  ：_/Player/Movement/player_move_context.h
// Date  ：2026/08/10
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorのコンテキストを定義するヘッダファイル
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_settings_asset.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/Move/player_move_motor.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/Move/player_move_rotate.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/Move/player_move_effects.h"

/// @brief PlayerMoveのランタイム状態を保持する構造体
struct PlayerMoveRuntimeState
{
    XMFLOAT3    m_controlVelocity = { 0.0f, 0.0f, 0.0f };   // 自身の移動要求による速度
    XMFLOAT3    m_physicsVelocity = { 0.0f, 0.0f, 0.0f };   // 外部要因による速度（衝突や押し出しなど）
    XMFLOAT3    m_desiredPosition = { 0.0f, 0.0f, 0.0f };   // 次フレームの目標位置

    bool    m_isGrounded = false; // 地面接地フラグ

    float   m_currentAngleY = 0.0f;
};

/// @brief PlayerMoveBehaviorのコンテキストを保持する構造体
struct PlayerMoveContext
{
    class PlayerMoveBehavior* owner = nullptr;

    PlayerMoveRuntimeState runtimeState;
    const PlayerMoveSettingsAsset* settingsAsset = nullptr;
    const auto& settings() {
        return settingsAsset->GetData();
    }

    class TransformComponent* transform = nullptr;
    class RigidbodyComponent* rigidbody = nullptr;
    class BoxColliderComponent* collider = nullptr;

    PlayerMoveMotor moveMotor;
    PlayerMoveRotate moveRotate;
    PlayerMoveEffects moveEffects;
};
