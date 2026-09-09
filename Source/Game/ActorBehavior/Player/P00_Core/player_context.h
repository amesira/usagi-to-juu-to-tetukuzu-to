//---------------------------------------------------
// File  ：_/ActorBehavior/Player/P00_Core/player_context.h
// Date  ：2026/07/30
// Author：Miu Kitamura
// 
// ・プレイヤーの状態や設定を管理するコンテキストクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

struct PlayerRuntimeState {
    DirectX::XMFLOAT3 m_controlVelocity{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_physicsVelocity{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_desiredPosition{ 0.0f, 0.0f, 0.0f };
    bool m_isGrounded = false;

    /// @brief プレイヤーの右方向をX、前方向をYとした移動入力
    DirectX::XMFLOAT2 localMoveParameter{ 0.0f, 0.0f };
    /// @brief カメラの右方向をX、前方向をYとした移動入力
    DirectX::XMFLOAT2 cameraBaseMoveParameter{ 0.0f, 0.0f };
};

/// @brief プレイヤーの状態や設定を管理するコンテキストクラス
class PlayerContext {
public:
    class PlayerBehavior* owner = nullptr;
    class IScene* scene = nullptr;
    class TransformComponent* transform = nullptr;

    PlayerRuntimeState runtimeState;

    class PlayerLocomotionController* locomotionController = nullptr;
    class PlayerConditionMachine* conditionMachine = nullptr;
    class PlayerActionMachine* actionMachine = nullptr;
    class PlayerWeaponController* weaponController = nullptr;
    class PlayerAnimationController* animationController = nullptr;

    class PlayerMoveBehavior* moveBehavior = nullptr;

    class PlayerUiBehavior* uiBehavior = nullptr;

    class CameraComponent* mainCamera = nullptr;
    TransformComponent* mainCameraTransform = nullptr;

    class CameraControlBehavior* cameraControlBehavior = nullptr;
    const class CameraSettingsAsset* defaultCameraSettingsAsset = nullptr;

    // FIX: この下にDataAssetで作成したSettingsを入れる予定
};
