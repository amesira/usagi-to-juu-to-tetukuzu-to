//===================================================
// File  ：_/Player/P10_Locomotion/player_locomotion_controller.cpp
// Date  ：2026/08/07
// Author：Miu Kitamura
// 
// ・プレイヤーの移動挙動を制御するコントローラークラス
//===================================================
#include "player_locomotion_controller.h"

// engine
#include "Engine/Component/transform_component.h"
#include "Engine/Component/camera_component.h"

// game
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

namespace {
    // === 入力構築のためのヘルパー関数 ===
    /// @brief DirectionSourceInfoに基づいて方向を計算する
    XMFLOAT3 CalculateDirection(
        const PlayerLocomotionController::DirectionSourceInfo& dirSourceInfo,
        const PlayerContext& context,
        const PlayerInput& input
    )
    {
        XMFLOAT3 dir = { 0.0f, 0.0f, 0.0f };

        switch (dirSourceInfo.source) {
            case PlayerLocomotionController::DirectionSource::MoveInput: {
                dir = input.moveDirection;
                break;
            }
            case PlayerLocomotionController::DirectionSource::CameraForward: {
                if (context.owner && context.mainCamera) {
                    XMFLOAT3 forward = context.mainCamera->GetForward();
                    dir = forward;
                }
                break;
            }
            case PlayerLocomotionController::DirectionSource::LookAtTarget: {
                if (dirSourceInfo.targetTransform && context.transform) {
                    XMFLOAT3 targetPos = dirSourceInfo.targetTransform->GetPosition();
                    XMFLOAT3 playerPos = context.transform->GetPosition();
                    dir = MiMath::Subtract(targetPos, playerPos);
                }
                break;
            }
            case PlayerLocomotionController::DirectionSource::FixedDirection: {
                dir = dirSourceInfo.fixedDirection;
                break;
            }
        }

        dir.y = 0.0f; // Y軸方向は無視
        return MiMath::Normalize(dir);
    }
}

// ---------------------------------------------------

/// @brief 初期化処理
void PlayerLocomotionController::Initialize()
{
    for (int i = 0; i < 8; i++) {
        m_locomotionRequests[i] = nullptr;
    }
}

/// @brief 終了処理
void PlayerLocomotionController::Finalize()
{
    for (int i = 0; i < 8; i++) {
        m_locomotionRequests[i] = nullptr;
    }
}

/// @brief プレイヤーの移動・回転意図を構築する
PlayerMoveIntent PlayerLocomotionController::BuildIntent(const PlayerContext& context, const PlayerInput& input)
{
    // 最も優先度の高いLocomotionRequestを取得する
    const LocomotionRequest* selectedRequest = nullptr;
    for (int i = 0; i < 8; i++) {
        if (m_locomotionRequests[i] != nullptr) {
            if (selectedRequest == nullptr || m_locomotionRequests[i]->priority > selectedRequest->priority) {
                selectedRequest = m_locomotionRequests[i];
            }
        }
    }

    // 選択されたLocomotionRequestがない場合はデフォルトのリクエストを使用する
    if (selectedRequest == nullptr) {
        selectedRequest = LocomotionRequest::Default();
    }

    // 移動方向と回転方向を計算する
    XMFLOAT3 moveDirection = CalculateDirection(selectedRequest->moveDirSourceInfo, context, input);
    XMFLOAT3 rotateDirection = CalculateDirection(selectedRequest->rotateDirSourceInfo, context, input);

    // === PlayerMoveIntentを構築して返す ===
    PlayerMoveIntent intent = {};

    // 方向は正規化し、入力自体の大きさはmoveInputMagnitudeで保持する
    intent.moveDirection = moveDirection;
    intent.moveInputMagnitude = MiMath::Length(input.moveInput);
    if (intent.moveInputMagnitude > 1.0f) {
        intent.moveInputMagnitude = 1.0f; // 入力の大きさを1.0fに制限
    }

    intent.rotateDirection = rotateDirection;

    intent.speedMultiplier = selectedRequest->speedMultiplier;
    intent.jumpPowerMultiplier = selectedRequest->jumpPowerMultiplier;

    intent.pauseMovement = selectedRequest->pauseMovement;
    intent.canMove = selectedRequest->canMove;
    intent.canRotate = selectedRequest->canRotate;
    intent.useGravity = selectedRequest->useGravity;
    intent.canJump = selectedRequest->canJump;
    intent.applyRotateRightNow = selectedRequest->applyRotateRightNow;

    // === ForceIntentの設定 ===
    intent.forceMoveIntent.isActive = (m_forceMoveRequest.priority > 0);
    intent.forceMoveIntent.targetPosition = m_forceMoveRequest.targetPosition;
    intent.forceRotateIntent.isActive = (m_forceRotateRequest.priority > 0);
    intent.forceRotateIntent.targetDirection = CalculateDirection(m_forceRotateRequest.directionSourceInfo, context, input);
    if (MiMath::Length(intent.forceRotateIntent.targetDirection) < 0.001f) {
        intent.forceRotateIntent.isActive = false; // 方向が無効な場合は強制回転を無効化
    }

    m_forceMoveRequest.priority = -1;
    m_forceRotateRequest.priority = -1;

    return intent;
}
