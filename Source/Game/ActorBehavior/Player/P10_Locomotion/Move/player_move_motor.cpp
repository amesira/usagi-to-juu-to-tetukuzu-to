//===================================================
// File  ：_/Player/Movement/player_move_motor.cpp
// Date  ：2026/07/27
// Author：Miu Kitamura
// 
// ・PlayerMoveBehaviorの移動処理を担当するクラス
//===================================================
#include "player_move_motor.h"

#include "Utility/utility_master.h"

#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Framework/Component/rigidbody_component.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"

/// @brief PlayerMoveMotorの移動処理を更新する
void PlayerMoveMotor::UpdateMove_Motor(PlayerMoveContext& context, const PlayerMoveIntent& intent, float deltaTime)
{
    if (intent.useGravity) {
        if (context.runtimeState.m_isGrounded) {
            context.runtimeState.m_physicsVelocity.y = -0.1f;
        }
        else {
            float gravityScale = 1.0f; // 重力のスケール（必要に応じて調整可能）

            // 上昇中
            if (context.runtimeState.m_physicsVelocity.y > 0.01f) {
                gravityScale = 1.0f;
            }
            // 下降中
            else if (context.runtimeState.m_physicsVelocity.y < -0.01f) {
                gravityScale = 1.5f;
            }
            // 頂上付近
            else {
                gravityScale = 0.3f;
            }

            // 重力加速度を計算する
            float gravityAcceleration = -9.81f * gravityScale;
            context.runtimeState.m_physicsVelocity.y += gravityAcceleration * deltaTime;
        }
    }

    if (intent.canMove) {

        XMFLOAT3 inputVelocity = MiMath::Multiply(intent.moveDirection, intent.moveInputMagnitude * context.settings().moveSpeed * intent.speedMultiplier);
        inputVelocity.y = 0.0f;

        // 移動入力がある場合は通常の平滑化時間、移動入力がない場合は停止時の平滑化時間を使用する
        float smoothTime = (intent.moveInputMagnitude > 0.01f) ? context.settings().smoothTime : context.settings().stopSmoothTime;

        // SmoothDampを使用して、現在の速度から目標速度に向かって平滑化する
        float desiredVelocityX = MiMath::SmoothDamp(
            context.runtimeState.m_controlVelocity.x, 
            inputVelocity.x, 
            m_velocitySmoothDamp.x, // REF: 速度保持変数を参照渡し
            smoothTime, 
            deltaTime);

        float desiredVelocityZ = MiMath::SmoothDamp(
            context.runtimeState.m_controlVelocity.z, 
            inputVelocity.z, 
            m_velocitySmoothDamp.z, // REF: 速度保持変数を参照渡し
            smoothTime, 
            deltaTime);

        // === ControlVelocityを更新する ===
        context.runtimeState.m_controlVelocity.x = desiredVelocityX;
        context.runtimeState.m_controlVelocity.z = desiredVelocityZ;
    }


}
