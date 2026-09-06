//---------------------------------------------------
// File  ：_/Player/P10_Locomotion/player_move_intent.h
// Date  ：2026/08/07
// Author：Miu Kitamura
// 
// ・Playerの移動意図を表す構造体
//---------------------------------------------------
#pragma once
#include "Utility/utility_master.h"

struct ForceMoveIntent
{
    bool isActive = false; // 強制移動が有効かどうか
    XMFLOAT3 targetPosition = { 0.0f, 0.0f, 0.0f }; // 強制移動の目標位置
};

struct ForceRotateIntent
{
    bool isActive = false; // 強制回転が有効かどうか
    XMFLOAT3 targetDirection = { 0.0f, 0.0f, 1.0f }; // 強制回転の目標方向（正規化済み）
};

/// @brief プレイヤーの移動意図を表す構造体
struct PlayerMoveIntent 
{
    XMFLOAT3 moveDirection = { 0.0f, 0.0f, 0.0f }; // 移動方向（正規化済み）
    float moveInputMagnitude = 1.0f; // 移動入力の大きさ（0.0f～1.0f）
    XMFLOAT3 rotateDirection = { 0.0f, 0.0f, 1.0f }; // 回転方向（正規化済み）
    
    float speedMultiplier = 1.0f; // 移動速度の倍率
    float jumpPowerMultiplier = 1.0f; // ジャンプ力の倍率

    bool pauseMovement = false; // 移動を一時停止するかどうか

    bool canMove = true;   // 移動可能かどうか
    bool canRotate = true; // 回転可能かどうか
    bool useGravity = true;  // 重力を使用するかどうか
    bool canJump = true;     // ジャンプ可能かどうか

    bool applyRotateRightNow = false; // 即座に回転を適用するかどうか

    ForceMoveIntent forceMoveIntent; // 強制移動の意図
    ForceRotateIntent forceRotateIntent; // 強制回転の意図
};
