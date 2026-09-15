//---------------------------------------------------
// File  ：_/E10_Locomotion/enemy_locomotion_controller.h
// Date  ：2026/09/10
// Author：Miu Kitamura
// 
// ・Enemyの移動要求を複数の機能から受け取り、優先度で解決する。
// ・PlayerLocomotionControllerとほぼ一緒。
// FIX: 移動要求はハンドル経由。こっちにプレイヤーも合わせたい
//---------------------------------------------------
#pragma once
#include <array>
#include <DirectXMath.h>

#include "enemy_move_intent.h"

class EnemyContext;

/// @brief 複数機能から届く移動要求を優先度で解決する。
class EnemyLocomotionController {
public:
    static constexpr int INVALID_REQUEST_HANDLE = -1;
    static constexpr int MAX_REQUEST_COUNT = 8;

    enum class DirectionSource {
        None,
        FixedDirection,
        TargetPosition,
    };

    struct DirectionSourceInfo {
        DirectionSource source = DirectionSource::None;
        DirectX::XMFLOAT3 fixedDirection = {};
        DirectX::XMFLOAT3 targetPosition = {};
    };

    struct LocomotionRequest {
        int priority = 0;
        DirectionSourceInfo moveDirection;
        DirectionSourceInfo rotateDirection;
        float moveSpeedMultiplier = 1.0f;
        float rotationSpeedMultiplier = 1.0f;
        EnemyMovementMode movementMode = EnemyMovementMode::ControlVelocity;
        bool canRotate = true;
        bool canMove = true;
        bool useGravity = true;
    };

    struct ForceMoveRequest {
        int priority = 0;
        DirectX::XMFLOAT3 targetPosition = {};
    };

private:
    struct RequestSlot {
        bool active = false;
        LocomotionRequest request;
    };

    std::array<RequestSlot, MAX_REQUEST_COUNT> m_requests = {};
    ForceMoveRequest m_forceMoveRequest;
    bool m_hasForceMoveRequest = false;

public:
    void Initialize();
    void Finalize();

    /// @brief 複数の移動要求から優先度の高いものを選択し、EnemyMoveIntentを構築する
    EnemyMoveIntent BuildIntent(const EnemyContext& context);

    // 1フレーム限定。同優先度では先に追加した要求を維持する。
    void AddForceMoveRequest(const ForceMoveRequest& request) {
        if (!m_hasForceMoveRequest || request.priority > m_forceMoveRequest.priority) {
            m_forceMoveRequest = request;
            m_hasForceMoveRequest = true;
        }
    }

    // === LocomotionRequestの管理 ===
    int AddRequest(const LocomotionRequest& request) {
        for (int i = 0; i < MAX_REQUEST_COUNT; i++) {
            if (!m_requests[i].active) {
                m_requests[i].active = true;
                m_requests[i].request = request;
                return i;
            }
        }
        return INVALID_REQUEST_HANDLE;
    }
    bool UpdateRequest(int handle, const LocomotionRequest& request) {
        for (int i = 0; i < MAX_REQUEST_COUNT; i++) {
            if (m_requests[i].active && i == handle) {
                m_requests[i].request = request;
                return true;
            }
        }
        return false;
    }
    void RemoveRequest(int handle) {
        for (int i = 0; i < MAX_REQUEST_COUNT; i++) {
            if (m_requests[i].active && i == handle) {
                m_requests[i].active = false;
                return;
            }
        }
    }
    void ClearRequests(){
        m_forceMoveRequest = {};
        m_hasForceMoveRequest = false;
        for (int i = 0; i < MAX_REQUEST_COUNT; i++) {
            m_requests[i].active = false;
        }
    }

};
