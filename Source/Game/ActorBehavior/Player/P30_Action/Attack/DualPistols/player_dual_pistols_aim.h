//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_aim.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃の照準処理
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>

struct PlayerDualPistolsContext;

class PlayerDualPistolsAim {
public:
    struct AimResult {
        DirectX::XMFLOAT3 cameraRayOrigin{};
        DirectX::XMFLOAT3 cameraRayDirection{};
        DirectX::XMFLOAT3 targetPosition{};

        DirectX::XMFLOAT3 leftMuzzlePosition{};
        DirectX::XMFLOAT3 rightMuzzlePosition{};
        DirectX::XMFLOAT3 leftFireDirection{};
        DirectX::XMFLOAT3 rightFireDirection{};

        bool hasTargetHit = false;
        bool hasLeftMuzzle = false;
        bool hasRightMuzzle = false;
    };

private:
    AimResult m_aimResult{};

public:
    void Initialize(PlayerDualPistolsContext& context);
    void EnterAim(PlayerDualPistolsContext& context);
    void UpdateAim(PlayerDualPistolsContext& context, float deltaTime);
    void ExitAim(PlayerDualPistolsContext& context);

    const AimResult& GetAimResult() const { return m_aimResult; }

};
