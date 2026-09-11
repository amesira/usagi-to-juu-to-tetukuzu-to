//===================================================
// File  ：_/E10_Locomotion/Move/enemy_move_motor.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_move_motor.h"

#include "Utility/utility_master.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move_context.h"
#include "Game/ActorBehavior/Enemy/E10_Locomotion/enemy_move_intent.h"

/// @brief EnemyMoveMotorの移動処理を更新する
void EnemyMoveMotor::UpdateMotor(EnemyMoveContext& context, const EnemyMoveIntent& intent, float deltaTime)
{
    if (!intent.useGravity) {
        context.runtimeState.physicsVelocity.y = 0.0f;
    }
    else if (context.runtimeState.isGrounded) {
        context.runtimeState.physicsVelocity.y = -0.1f;
    }
    else {
        float normalizedYVelocity = context.runtimeState.physicsVelocity.y / 0.1f;

        // 重力加速度を計算する
        float gravityAcceleration = -9.81f;
        context.runtimeState.physicsVelocity.y += gravityAcceleration * deltaTime;
    }

    if (intent.canMove && intent.movementMode == EnemyMovementMode::ControlVelocity)
    {
        // 移動速度を計算する
        float moveSpeed = context.settings().moveSpeed * intent.moveSpeedMultiplier;

        // 移動入力速度を算出
        m_inputVelocity = MiMath::Multiply(intent.moveDirection, moveSpeed);
        m_inputVelocity.y = 0.0f;

        // 状態に応じた平滑化時間を計算する
        float smoothTime = CalculateSmoothTime(context);

        // === 目標速度を更新する ===
        UpdateDesiredVelocity(context, smoothTime, deltaTime);

        // === ControlVelocityを更新する ===
        context.runtimeState.controlVelocity.x = m_desiredVelocity.x;
        context.runtimeState.controlVelocity.z = m_desiredVelocity.z;
    }
    else {
        // 移動不可の場合は移動入力速度をゼロにする
        m_inputVelocity = { 0.0f, 0.0f, 0.0f };
        m_desiredVelocity = {};
        m_velocitySmoothDamp = {};
        context.runtimeState.controlVelocity.x = 0.0f;
        context.runtimeState.controlVelocity.z = 0.0f;

        if (intent.movementMode == EnemyMovementMode::StopHorizontal) {
            context.runtimeState.physicsVelocity.x = 0.0f;
            context.runtimeState.physicsVelocity.z = 0.0f;
        }
    }
}

/// @brief 現在の状況に応じた平滑化時間を計算する
float EnemyMoveMotor::CalculateSmoothTime(EnemyMoveContext& context)
{
    bool stopInput = (MiMath::Length(m_inputVelocity) < 0.01f);

    if (!stopInput) {
        return context.settings().smoothTime;
    }
    else {
        return context.settings().stopSmoothTime;
    }
}

/// @brief 滑らかにするための目標速度を計算する
void EnemyMoveMotor::UpdateDesiredVelocity(EnemyMoveContext& context, float smoothTime, float deltaTime)
{
    // SmoothDampを使用して、現在の速度から目標速度に向かって平滑化する
    float desiredVelocityX = MiMath::SmoothDamp(
        m_desiredVelocity.x,
        m_inputVelocity.x,
        m_velocitySmoothDamp.x, // REF: 速度保持変数を参照渡し
        smoothTime,
        deltaTime);

    float desiredVelocityZ = MiMath::SmoothDamp(
        context.runtimeState.controlVelocity.z,
        m_inputVelocity.z,
        m_velocitySmoothDamp.z, // REF: 速度保持変数を参照渡し
        smoothTime,
        deltaTime);

    m_desiredVelocity.x = desiredVelocityX;
    m_desiredVelocity.z = desiredVelocityZ;
}
