//---------------------------------------------------
// File  ：_/P30_Action/Attack/Shotgun/player_shotgun_action.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・ショットガン攻撃アクション
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Player/P30_Action/player_action_base.h"
#include "player_shotgun_context.h"

class PlayerShotgunAction : public PlayerActionBase {
private:
    PlayerShotgunContext m_context;

    bool m_enteredPhase = false;
    bool m_enteredAnimationSubMachine = false;

public:
    PlayerShotgunAction() : PlayerActionBase("Shotgun", ActionCategory::Attack, 10, true) {}
    void Initialize(
        const PlayerContext& context,
        PlayerShotgunSettingsAsset* settingsAsset,
        const class CameraSettingsAsset* cameraSettingsAsset);

    bool CanStart(const PlayerContext& context, const PlayerInput& input) override;

    void Start(PlayerContext& context, const PlayerInput& input) override;
    void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) override;
    void Finish(PlayerContext& context, const PlayerInput& input) override;

private:
    void UpdateMuzzleState();
    void ChangePhase(PlayerShotgunRuntimeState::Phase newPhase);

};
