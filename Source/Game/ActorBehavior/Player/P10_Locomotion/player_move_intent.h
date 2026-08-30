//---------------------------------------------------
// File  ：_/Player/P10_Locomotion/player_move_intent.h
// Date  ：2026/08/07
// Author：Miu Kitamura
// 
// ・プリヤ―の移動意図を表す構造体
//---------------------------------------------------
#pragma once
#include "Utility/utility_master.h"

/// @brief プレイヤーの移動意図を表す構造体
struct PlayerMoveIntent 
{
    XMFLOAT3 moveDirection = { 0.0f, 0.0f, 0.0f }; // 移動方向（正規化済み）
    float moveInputMagnitude = 1.0f; // 移動入力の大きさ（0.0f～1.0f）
    XMFLOAT3 rotateDirection = { 0.0f, 0.0f, 1.0f }; // 回転方向（正規化済み）
    
    float speedMultiplier = 1.0f; // 移動速度の倍率
    float jumpPowerMultiplier = 1.0f; // ジャンプ力の倍率

    bool canMove = true;   // 移動可能かどうか
    bool canRotate = true; // 回転可能かどうか
    bool useGravity = true;  // 重力を使用するかどうか
    bool canJump = true;     // ジャンプ可能かどうか

    bool applyRotateRightNow = false; // 即座に回転を適用するかどうか
};