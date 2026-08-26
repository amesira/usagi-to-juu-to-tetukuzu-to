//---------------------------------------------------
// File  ：_/ActorBehavior/Player/P00_Core/player_context.h
// Date  ：2026/07/30
// Author：Miu Kitamura
// 
// ・プレイヤーの状態や設定を管理するコンテキストクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

/// @brief プレイヤーの状態や設定を管理するコンテキストクラス
class PlayerContext {
public:
    class PlayerBehavior* owner = nullptr;
    class IScene* scene = nullptr;
    class TransformComponent* transform = nullptr;

    class PlayerLocomotionController* locomotionController = nullptr;
    class PlayerConditionMachine* conditionMachine = nullptr;
    class PlayerActionMachine* actionMachine = nullptr;
    class PlayerWeaponController* weaponController = nullptr;

    class PlayerMoveBehavior* moveBehavior = nullptr;

    class CameraComponent* mainCamera = nullptr;
    TransformComponent* mainCameraTransform = nullptr;

    class CameraControlBehavior* cameraControlBehavior = nullptr;

    // FIX: この下にDataAssetで作成したSettingsを入れる予定
};
