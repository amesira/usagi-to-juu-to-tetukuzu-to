//---------------------------------------------------
// File  ：.../ActorBehavior/Player/player_input.h
// Date  ：2026/07/30
// Author：Miu Kitamura
// 
// ・プレイヤーの入力を管理する構造体
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

class PlayerInput {
public:
    float horizontal;   // 水平方向の入力
    float vertical;     // 垂直方向の入力
    DirectX::XMFLOAT3 moveInput;
    DirectX::XMFLOAT3 moveDirection; // 移動入力（horizontal, verticalをカメラから変換）

    // ジャンプ入力
    bool    triggerJumpCommand;

    // ダッシュ入力
    bool    triggerDashCommand;

    // エイム入力
    bool    triggerAimCommand;
    bool    holdAimCommand;
    bool    releaseAimCommand;

    // 攻撃入力
    bool    triggerAttackCommand;
    bool    holdAttackCommand;
    bool    releaseAttackCommand;
};