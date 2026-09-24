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
    bool m_enteredAnimationSubMachine = false;

public:
    PlayerDualPistolsAction() : PlayerActionBase("DualPistols", ActionCategory::Attack, 10, true) {}
    void Initialize(const PlayerContext& context, PlayerDualPistolsSettingsAsset* settingsAsset);

    bool CanStart(const PlayerContext& context, const PlayerInput& input) override;
    bool IsReleaseAttackInput(const PlayerInput& input) const;

    void Start(PlayerContext& context, const PlayerInput& input) override;
    void Update(PlayerContext& context, const PlayerInput& input, float deltaTime) override;
    void Finish(PlayerContext& context, const PlayerInput& input) override;

    void UpdateBackground(PlayerContext& context, const PlayerInput& input, float deltaTime) override;

private:
    void UpdateMuzzleStates();
    void ChangePhase(PlayerDualPistolsRuntimeState::Phase newPhase);

    // === アニメーション・UIの更新処理（未実装） ===
    void UpdateAnimation(PlayerContext& context, float deltaTime) {}
    void UpdateUi(PlayerContext& context, float deltaTime) {}

};
