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

    XMFLOAT3 m_inputVelocity = { 0.0f, 0.0f, 0.0f }; // 入力による目標速度を保持する変数
    XMFLOAT3 m_desiredVelocity = { 0.0f, 0.0f, 0.0f }; // 目標速度を保持する変数

public:
    /// @brief PlayerMoveMotorの移動処理を更新する
    void UpdateMotor(struct PlayerMoveContext& context, const struct PlayerMoveIntent& intent, float deltaTime);

private:
    /// @brief 現在の状況に応じた平滑化時間を計算する
    float CalculateSmoothTime(struct PlayerMoveContext& context);
    /// @brief 目標位置を更新
    void UpdateDesiredVelocity(struct PlayerMoveContext& context, float smoothTime, float deltaTime);

};