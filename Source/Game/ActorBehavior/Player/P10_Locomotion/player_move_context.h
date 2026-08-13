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

/// @brief PlayerMoveの参照情報を保持する構造体
struct PlayerMoveReferences
{

};

/// @brief PlayerMoveの設定値を保持する構造体（FIX: いずれはDataAsset化する）
struct PlayerMoveSettings
{
    float   moveSpeed = 10.0f;
    float   jumpForce = 10.0f;

    float   rotationSpeed = 10.0f;

    float   smoothTime = 0.1f; // 平滑化の時間（秒）
    float   stopSmoothTime = 0.05f; // 停止時の平滑化の時間（秒）
};

/// @brief PlayerMoveBehaviorのコンテキストを保持する構造体
struct PlayerMoveContext
{
    class PlayerMoveBehavior* owner = nullptr;

    PlayerMoveRuntimeState runtimeState;
    PlayerMoveReferences references;
    PlayerMoveSettings settings;

    class TransformComponent* transform = nullptr;
    class RigidbodyComponent* rigidbody = nullptr;

    PlayerMoveMotor moveMotor;
    PlayerMoveRotate moveRotate;
    PlayerMoveEffects moveEffects;
};