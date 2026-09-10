#pragma once

#include <DirectXMath.h>

/// @brief LocomotionControllerがMoveBehaviorへ渡す、1フレーム分の移動意図。
struct EnemyMoveIntent {
    DirectX::XMFLOAT3 moveDirection = {};
    DirectX::XMFLOAT3 rotateDirection = { 0.0f, 0.0f, 1.0f };
    float moveSpeed = 0.0f;
    float rotationSpeed = 10.0f;
    bool canMove = true;
    bool canRotate = true;
};
