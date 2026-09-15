//---------------------------------------------------
// File  ：_/E10_Locomotion/Move/enemy_move_motor.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・EnemyMoveの移動処理を担当するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/ControllerBehavior/StageBounds/stage_bounds_resolver.h"
using namespace DirectX;

class EnemyMoveMotor {
private:
    // SmoothDamp用の速度保持変数
    XMFLOAT3 m_velocitySmoothDamp = { 0.0f, 0.0f, 0.0f };

    XMFLOAT3 m_inputVelocity = { 0.0f, 0.0f, 0.0f }; // 入力による目標速度を保持する変数
    XMFLOAT3 m_desiredVelocity = { 0.0f, 0.0f, 0.0f }; // 目標速度を保持する変数

public:
    void ApplyBounds(const StageBounds::Result& result) {
        result.ClipVelocity(m_inputVelocity);
        result.ClipVelocity(m_desiredVelocity);
        result.ClipVelocity(m_velocitySmoothDamp);
    }
    /// @brief EnemyMoveMotorの移動処理を更新する
    void UpdateMotor(struct EnemyMoveContext& context, const struct EnemyMoveIntent& intent, float deltaTime);

private:
    /// @brief 現在の状況に応じた平滑化時間を計算する
    float CalculateSmoothTime(struct EnemyMoveContext& context);
    /// @brief 目標位置を更新
    void UpdateDesiredVelocity(struct EnemyMoveContext& context, float smoothTime, float deltaTime);

};