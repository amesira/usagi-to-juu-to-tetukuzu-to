// enemy_move_intent.h
// 2026/09/10
#pragma once

#include <DirectXMath.h>

enum class EnemyMovementMode {
    ControlVelocity,       // Motorが通常の移動速度を計算する
    StopHorizontal,        // 水平速度を即座に停止する
    KeepRigidbodyVelocity, // ノックバックなど、外部から設定された速度を維持する
};

/// @brief LocomotionControllerがEnemyMoveへ渡す、1フレーム分の移動意図。
struct EnemyMoveIntent {
    struct ForceMoveIntent {
        bool isActive = false;
        DirectX::XMFLOAT3 targetPosition = {};
    };
    ForceMoveIntent forceMoveIntent;
    DirectX::XMFLOAT3 moveDirection = {};
    DirectX::XMFLOAT3 rotateDirection = { 0.0f, 0.0f, 1.0f };

    float moveSpeedMultiplier = 1.0f;
    float rotationSpeedMultiplier = 1.0f;

    EnemyMovementMode movementMode = EnemyMovementMode::StopHorizontal;
    bool canRotate = true;
    bool canMove = true; // 通常の移動入力を許可する。強制移動とは独立。
    bool useGravity = true;
};
