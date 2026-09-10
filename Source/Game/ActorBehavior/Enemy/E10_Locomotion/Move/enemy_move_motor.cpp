//===================================================
// File  ：_/Player/Movement/player_move_motor.cpp
// Date  ：2026/07/27
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動処理を担当するクラス
//===================================================
#include "enemy_move_motor.h"

#include "Utility/utility_master.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"

/// @brief PlayerMoveMotorの移動処理を更新する
void PlayerMoveMotor::UpdateMotor(PlayerMoveContext& context, const PlayerMoveIntent& intent, float deltaTime)
{
    if (intent.useGravity) {
        if (context.runtimeState.m_isGrounded) {
            context.runtimeState.m_physicsVelocity.y = -0.1f;
        }
        else {
            float normalizedYVelocity = context.runtimeState.m_physicsVelocity.y / 0.1f;
            float gravityScale = context.settings().gravityScale.Evaluate(normalizedYVelocity);

            // 重力加速度を計算する
            float gravityAcceleration = -9.81f * gravityScale;
            context.runtimeState.m_physicsVelocity.y += gravityAcceleration * deltaTime;
        }
    }
    else {
        context.runtimeState.m_physicsVelocity.y = 0.0f;
    }

    if (intent.canMove)
    {
        // 移動速度を計算する
        float moveSpeed = context.settings().moveSpeed * intent.speedMultiplier * intent.moveInputMagnitude;
        if (!context.runtimeState.m_isGrounded) {
            moveSpeed *= context.settings().airSpeedMultiplier;
        }

        // 移動入力速度を算出
        m_inputVelocity = MiMath::Multiply(intent.moveDirection, moveSpeed);
        m_inputVelocity.y = 0.0f;

        // 状態に応じた平滑化時間を計算する
        float smoothTime = CalculateSmoothTime(context);

        // === 目標速度を更新する ===
        UpdateDesiredVelocity(context, smoothTime, deltaTime);

        // === ControlVelocityを更新する ===
        context.runtimeState.m_controlVelocity.x = m_desiredVelocity.x;
        context.runtimeState.m_controlVelocity.z = m_desiredVelocity.z;
    }
    else {
        // 移動不可の場合は速度をゼロにする
        m_inputVelocity = { 0.0f, 0.0f, 0.0f };
        m_desiredVelocity = { 0.0f, 0.0f, 0.0f };
        context.runtimeState.m_controlVelocity.x = 0.0f;
        context.runtimeState.m_controlVelocity.z = 0.0f;
    }
}

/// @brief 現在の状況に応じた平滑化時間を計算する
float PlayerMoveMotor::CalculateSmoothTime(PlayerMoveContext& context)
{
    bool stopInput = (MiMath::Length(m_inputVelocity) < 0.01f);

    if (context.runtimeState.m_isGrounded && !stopInput) {
        return context.settings().smoothTime;
    }
    else if (!context.runtimeState.m_isGrounded && !stopInput) {
        return context.settings().airSmoothTime;
    }
    else if (context.runtimeState.m_isGrounded && stopInput) {
        return context.settings().stopSmoothTime;
    }
    else if (!context.runtimeState.m_isGrounded && stopInput) {
        return context.settings().stopAirSmoothTime;
    }

    return context.settings().smoothTime;
}

/// @brief 滑らかにするための目標速度を計算する
void PlayerMoveMotor::UpdateDesiredVelocity(PlayerMoveContext& context, float smoothTime, float deltaTime)
{
    // SmoothDampを使用して、現在の速度から目標速度に向かって平滑化する
    float desiredVelocityX = MiMath::SmoothDamp(
        m_desiredVelocity.x,
        m_inputVelocity.x,
        m_velocitySmoothDamp.x, // REF: 速度保持変数を参照渡し
        smoothTime,
        deltaTime);

    float desiredVelocityZ = MiMath::SmoothDamp(
        context.runtimeState.m_controlVelocity.z,
        m_inputVelocity.z,
        m_velocitySmoothDamp.z, // REF: 速度保持変数を参照渡し
        smoothTime,
        deltaTime);

    m_desiredVelocity.x = desiredVelocityX;
    m_desiredVelocity.z = desiredVelocityZ;
}
