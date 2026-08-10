//---------------------------------------------------
// File  ：_/Player/Movement/player_move_motor.h
// Date  ：2026/07/27
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動処理を担当するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class PlayerMoveMotor {
private:
    // SmoothDamp用の速度保持変数
    XMFLOAT3 m_velocitySmoothDamp = { 0.0f, 0.0f, 0.0f };

public:
    /// @brief PlayerMoveMotorの移動処理を更新する
    void UpdateMove_Motor(struct PlayerMoveContext& context, const struct PlayerMoveIntent& intent, float deltaTime);


};