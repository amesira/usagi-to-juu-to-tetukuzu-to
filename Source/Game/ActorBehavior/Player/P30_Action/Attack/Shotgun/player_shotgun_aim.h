//---------------------------------------------------
// File  ：_/Shotgun/player_shotgun_aim.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・プレイヤーのショットガンの照準を制御するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"

struct PlayerShotgunContext;

class PlayerShotgunAim {
public:
    struct AimResult {
        DirectX::XMFLOAT3 cameraRayOrigin{};
        DirectX::XMFLOAT3 cameraRayDirection{};

        DirectX::XMFLOAT3 muzzlePosition{};
        DirectX::XMFLOAT3 targetPosition{};
        DirectX::XMFLOAT3 fireDirection{};

        bool hasTargetHit = false;
    };
private:
    bool m_isAiming = false;
    AimResult m_aimResult = {};

    int m_locomotionRequestID = -1;
    PlayerLocomotionController::LocomotionRequest m_locomotionRequest;

    DirectX::XMFLOAT2 m_moveBlendParameter = {};
    DirectX::XMFLOAT2 m_moveBlendParameterVelocity = {};

public:
    void EnterAim(PlayerShotgunContext& context);
    void UpdateAim(PlayerShotgunContext& context, float deltaTime);
    void ExitAim(PlayerShotgunContext& context);

    const AimResult& GetAimResult() const { return m_aimResult; }

    /// @brief エイムモードのためのカメラを設定する
    void SetAimingCameraSetting(PlayerShotgunContext& context, bool enable);

    /// @brief エイム中のアニメーションを更新する
    void UpdateAimingAnimation(PlayerShotgunContext& context, float deltaTime);

};
