//===================================================
// File  ：_/Player/P10_Locomotion/player_locomotion_controller.cpp
// Date  ：2026/08/07
// Author：Miu Kitamura
// 
// ・プレイヤーの移動挙動を制御するコントローラークラス
//===================================================
#include "player_locomotion_controller.h"

// engine
#include "Engine/Framework/Component/transform_component.h"

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
                if (context.owner && context.mainCameraTransform) {
                    XMFLOAT3 forward = context.mainCameraTransform->GetForward();
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

/// @brief 新しいLocomotionRequestを追加する
int PlayerLocomotionController::AddLocomotionRequest(const LocomotionRequest& request)
{
    for (int i = 0; i < 8; i++) {
        if (m_locomotionRequests[i] == nullptr) {
            m_locomotionRequests[i] = &request;
            return i;
        }
    }

    return -1; // 空きがない場合は-1を返す
}

/// @brief 指定されたLocomotionRequestを削除する
void PlayerLocomotionController::RemoveLocomotionRequest(const LocomotionRequest& request)
{
    for (int i = 0; i < 8; i++) {
        if (m_locomotionRequests[i] == &request) {
            m_locomotionRequests[i] = nullptr;
            return;
        }
    }
}

/// @brief 指定されたインデックスのLocomotionRequestを削除する
void PlayerLocomotionController::RemoveLocomotionRequestByIndex(int index)
{
    for (int i = 0; i < 8; i++) {
        if (i == index) {
            m_locomotionRequests[i] = nullptr;
            return;
        }
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

    intent.canMove = selectedRequest->canMove;
    intent.canRotate = selectedRequest->canRotate;
    intent.useGravity = selectedRequest->useGravity;
    intent.applyRotateRightNow = selectedRequest->applyRotateRightNow;

    return intent;
}
