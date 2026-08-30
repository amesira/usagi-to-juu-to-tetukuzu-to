//---------------------------------------------------
// File  ：_/P30_Action/Attack/DualPistols/player_dual_pistols_action.h
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・二丁拳銃の攻撃アクション
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Player/P30_Action/player_action_base.h"
#include "player_dual_pistols_context.h"

class PlayerDualPistolsAction : public PlayerActionBase {
private:
    PlayerDualPistolsContext m_context;

    bool m_enteredPhase = false;
    bool m_enteredSlashBurstAnimationSubMachine = false;

public:
    PlayerDualPistolsAction() : PlayerActionBase("DualPistols", ActionCategory::Attack, 10, true) {}
    void Initialize(const PlayerContext& context, PlayerDualPistolsSettingsAsset* settingsAsset);

    bool CanStart(const PlayerContext& context, const PlayerInput& input) override;
    bool IsReleaseAttackInput(const PlayerInput& input) const;

    void Start(PlayerContext& context, const PlayerInput& input) override;
    void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) override;
    void Finish(PlayerContext& context, const PlayerInput& input) override;

private:
    void ChangePhase(PlayerDualPistolsRuntimeState::Phase newPhase);
};
