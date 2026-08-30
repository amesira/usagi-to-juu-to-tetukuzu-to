//---------------------------------------------------
// File  ：_/Player/P10_Locomotion/player_locomotion_controller.h
// Date  ：2026/08/07
// Author：Miu Kitamura
// 
// ・プレイヤーの移動挙動を制御するコントローラークラス
//---------------------------------------------------
#ifndef PLAYER_LOCOMOTION_CONTROLLER_H
#define PLAYER_LOCOMOTION_CONTROLLER_H
#include <DirectXMath.h>
using namespace DirectX;

#include "player_move_intent.h"

class PlayerLocomotionController {
public:
    /// @brief プレイヤーの移動・回転方向の参照元を指定する列挙型
    enum class DirectionSource 
    {
        MoveInput,      // 移動入力に基づく方向
        CameraForward,  // カメラの前方方向

        LookAtTarget,   // 指定されたターゲットを向く方向
        FixedDirection, // 固定された方向
    };

    /// @brief プレイヤーの移動・回転方向の参照情報を指定する構造体
    struct DirectionSourceInfo 
    {
        DirectionSource source = DirectionSource::MoveInput;

        // === Direction Source に応じた追加情報 ===
        class TransformComponent* targetTransform = nullptr;
        XMFLOAT3 fixedDirection = { 0.0f, 0.0f, 1.0f };
    };

    /// @brief プレイヤーの移動・回転のリクエスト情報を指定する構造体
    struct LocomotionRequest 
    {
        int priority = 0; // 優先度（数値が大きいほど優先度が高い）

        DirectionSourceInfo moveDirSourceInfo;   // 移動方向の参照情報
        DirectionSourceInfo rotateDirSourceInfo; // 回転方向の参照情報

        float speedMultiplier = 1.0f; // 移動速度の倍率
        float jumpPowerMultiplier = 1.0f; // ジャンプ力の倍率

        bool canMove = true;   // 移動可能かどうか
        bool canRotate = true; // 回転可能かどうか
        bool useGravity = true;  // 重力を使用するかどうか
        bool canJump = true;     // ジャンプ可能かどうか

        bool applyRotateRightNow = false; // 即座に回転を適用するかどうか

        /// @brief デフォルトのLocomotionRequestを生成する静的メソッド
        static LocomotionRequest* Default() {
            static LocomotionRequest* request = new LocomotionRequest();
            request->priority = 0;
            request->moveDirSourceInfo.source = DirectionSource::MoveInput;
            request->rotateDirSourceInfo.source = DirectionSource::MoveInput;
            request->speedMultiplier = 1.0f;
            request->canMove = true;
            request->canRotate = true;
            request->useGravity = true;
            request->applyRotateRightNow = false;
            return request;
        }
    };

private:
    const LocomotionRequest* m_locomotionRequests[8];

public:
    PlayerLocomotionController() = default;
    ~PlayerLocomotionController() = default;

    void Initialize();
    void Finalize();

    int AddLocomotionRequest(const LocomotionRequest& request) {
        for (int i = 0; i < 8; i++) {
            if (m_locomotionRequests[i] == nullptr) {
                m_locomotionRequests[i] = &request;
                return i;
            }
        }
        return -1; // 追加できなかった場合は-1を返す
    }

    void RemoveLocomotionRequest(const LocomotionRequest& request) {
        for (int i = 0; i < 8; i++) {
            if (m_locomotionRequests[i] == &request) {
                m_locomotionRequests[i] = nullptr;
                return;
            }
        }
    }
    void RemoveLocomotionRequestByIndex(int index) {
        if (index < 0 || index >= 8) return;
        m_locomotionRequests[index] = nullptr;
    }

    /// @brief プレイヤーの移動・回転意図を構築する
    PlayerMoveIntent BuildIntent(const class PlayerContext& context, const class PlayerInput& input);
};

#endif