#pragma once

#include <array>
#include <DirectXMath.h>

#include "enemy_move_intent.h"

class EnemyContext;

/// @brief 複数機能から届く移動要求を優先度で解決する。
class EnemyLocomotionController {
public:
    static constexpr int MAX_REQUEST_COUNT = 8;
    static constexpr int INVALID_REQUEST_HANDLE = -1;

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
        float moveSpeed = 0.0f;
        float rotationSpeed = 10.0f;
        bool canMove = true;
        bool canRotate = true;
    };

private:
    struct RequestSlot {
        bool active = false;
        LocomotionRequest request;
    };

    std::array<RequestSlot, MAX_REQUEST_COUNT> m_requests = {};

public:
    void Initialize();
    void Finalize();

    int AddRequest(const LocomotionRequest& request);
    bool UpdateRequest(int handle, const LocomotionRequest& request);
    void RemoveRequest(int handle);
    void ClearRequests();

    EnemyMoveIntent BuildIntent(const EnemyContext& context) const;
};
