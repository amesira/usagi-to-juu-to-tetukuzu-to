//===================================================
// File  ：_/P30_Action/Attack/Shotgun/player_shotgun_action.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・ショットガン攻撃アクション
//===================================================
#include "player_shotgun_action.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include "Engine/engine_service_locator.h"

namespace {
    using Phase = PlayerShotgunRuntimeState::Phase;
}

void PlayerShotgunAction::Initialize(const PlayerContext& context, PlayerShotgunReferences references, PlayerShotgunSettingsAsset* settingsAsset)
{
    m_context.owner = this;
    m_context.playerTransform = context.transform;
    m_context.cameraControlBehavior = context.cameraControlBehavior;
    m_context.locomotionController = context.locomotionController;
    m_context.weaponController = context.weaponController;

    m_context.runtimeState = {};
    m_context.references = references;
    m_context.settingsAsset = settingsAsset;
}

bool PlayerShotgunAction::CanStart(const PlayerContext& context, const PlayerInput& input)
{
    // エイム入力があるならショットガンアクションを開始できる
    return input.triggerAimCommand || input.holdAimCommand;
}

void PlayerShotgunAction::Start(PlayerContext& context, const PlayerInput& input)
{
    m_context.runtimeState.phase = PlayerShotgunRuntimeState::Phase::Entering;
    m_context.runtimeState.phaseTimer = 0.0f;

    m_enteredPhase = true;
}

void PlayerShotgunAction::Update(PlayerContext& context, const PlayerInput& input, float deltaTime)
{
    Phase currentPhase = m_context.runtimeState.phase;
    bool enteredPhase = m_enteredPhase;
    m_enteredPhase = false;

    switch (currentPhase) {
        case Phase::Entering: {
            // エイムへ行くまでの処理（アニメーションやエフェクトなど）
            ChangePhase(Phase::Aiming);
            break;
        }
        case Phase::Aiming: {
            if (enteredPhase) {
                m_context.aim.EnterAim(m_context);
            }

            m_context.aim.UpdateAim(m_context, deltaTime);

            if (input.triggerAttackCommand || input.holdAttackCommand) {
                ChangePhase(Phase::Charging);
            }
            break;
        }
        case Phase::Charging: {
           /* m_context.charging.Update(m_context, input, deltaTime);
            if (m_context.charging.IsChargeComplete()) {
                ChangePhase(Phase::Firing);
            }*/
            break;
        }
        case Phase::Firing: {
           // m_context.effects.Fire(m_context);
            ChangePhase(Phase::Recovery);
            break;
        }
        case Phase::Recovery: {
            
            break;
        }
        case Phase::Exiting: {
            m_context.aim.ExitAim(m_context);

            ChangePhase(Phase::None);
            break;
        }
        default: break;
    }
}

void PlayerShotgunAction::Finish(PlayerContext& context, const PlayerInput& input)
{

}

void PlayerShotgunAction::ChangePhase(PlayerShotgunRuntimeState::Phase newPhase)
{
    if (m_context.runtimeState.phase == newPhase) return;
    m_context.runtimeState.phase = newPhase;
    m_context.runtimeState.phaseTimer = 0.0f;
    m_enteredPhase = true;
}
