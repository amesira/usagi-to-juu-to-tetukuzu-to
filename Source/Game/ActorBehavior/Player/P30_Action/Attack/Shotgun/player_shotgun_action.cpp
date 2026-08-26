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
    m_context.scene = context.scene;
    m_context.playerTransform = context.transform;
    m_context.cameraTransform = context.mainCameraTransform;
    m_context.cameraComponent = context.mainCamera;
    m_context.cameraControlBehavior = context.cameraControlBehavior;
    m_context.locomotionController = context.locomotionController;
    m_context.weaponController = context.weaponController;

    m_context.runtimeState = {};
    m_context.references = references;
    m_context.settingsAsset = settingsAsset;
    m_context.effects.Initialize(m_context);
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
    // 終了チェック（仮）
    if (!input.holdAimCommand) {
        ChangePhase(Phase::Exiting);
    }

    Phase currentPhase = m_context.runtimeState.phase;
    bool enteredPhase = m_enteredPhase;
    m_enteredPhase = false;

    m_context.runtimeState.phaseTimer += deltaTime;

    switch (currentPhase) {
        case Phase::Entering: {
            if (enteredPhase) {
                m_context.aim.EnterAim(m_context);
            }

            if (m_context.runtimeState.phaseTimer >= m_context.settings().aimTransitionTime) {
                ChangePhase(Phase::Aiming);
            }
            break;
        }
        case Phase::Aiming: {
            m_context.aim.UpdateAim(m_context, deltaTime);

            if (input.triggerAttackCommand || input.holdAttackCommand) {
                ChangePhase(Phase::Charging);
            }
            break;
        }
        case Phase::Charging: {
            if (enteredPhase) {
                m_context.charging.Start(m_context);
            }
            m_context.charging.Update(m_context, deltaTime);

            if (!input.holdAttackCommand) {
                m_context.charging.Reset(m_context);
                ChangePhase(Phase::Firing);
            }
            break;
        }
        case Phase::Firing: {
            m_context.effects.PlayEffects(m_context, PlayerShotgunEffects::EffectsType::Fire);
            ChangePhase(Phase::Recovery);
            break;
        }
        case Phase::Recovery: {
            if (m_context.runtimeState.phaseTimer >= m_context.settings().recoveryTime) {
                ChangePhase(Phase::Aiming);
            }
            break;
        }
        case Phase::Exiting: {
            if (enteredPhase) {
                m_context.aim.ExitAim(m_context);
            }

            if (m_context.runtimeState.phaseTimer >= m_context.settings().aimTransitionTime) {
                ChangePhase(Phase::None);
                SetState(ActionState::WaitingToFinish);
            }
            break;
        }
        default: break;
    }
}

void PlayerShotgunAction::Finish(PlayerContext& context, const PlayerInput& input)
{
    // 割り込み終了でもカメラとLocomotionRequestを確実に復元する
    m_context.aim.ExitAim(m_context);
    m_context.effects.PlayEffects(m_context, PlayerShotgunEffects::EffectsType::ResetCharge);
    m_context.runtimeState = {};
    m_enteredPhase = false;
}

void PlayerShotgunAction::ChangePhase(PlayerShotgunRuntimeState::Phase newPhase)
{
    if (m_context.runtimeState.phase == newPhase) return;
    m_context.runtimeState.phase = newPhase;
    m_context.runtimeState.phaseTimer = 0.0f;
    m_enteredPhase = true;
}
