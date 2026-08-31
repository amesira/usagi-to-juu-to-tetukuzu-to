//===================================================
// File  ：_/P30_Action/Attack/DualPistols/player_dual_pistols_action.cpp
// Date  ：2026/08/26
// Author：Miu Kitamura
// 
// ・二丁拳銃の攻撃アクション
//===================================================
#include "player_dual_pistols_action.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

#include "Game/ActorBehavior/Player/player_behavior.h"
#include "Game/ActorBehavior/Player/player_animation_controller.h"
#include "Game/PresBehavior/Camera/camera_control_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Component/model_component.h"
#include "Engine/Graphics/model_animation_utility.h"

namespace {
    using Phase = PlayerDualPistolsRuntimeState::Phase;
}

void PlayerDualPistolsAction::Initialize(
    const PlayerContext& context,
    PlayerDualPistolsSettingsAsset* settingsAsset)
{
    m_context.owner = this;
    m_context.scene = context.scene;
    m_context.settingsAsset = settingsAsset;

    m_context.playerTransform = context.transform;
    m_context.playerRuntimeState = &context.runtimeState;
    GameObject* player = context.owner ? context.owner->GetOwner() : nullptr;
    m_context.playerModel = player ? player->GetComponent<ModelComponent>() : nullptr;
    if (m_context.playerModel) {
        ModelAnimationUtility::FindBoneIndex(
            *m_context.playerModel,
            "Gun.L",
            m_context.references.gunLBoneIndex);
        ModelAnimationUtility::FindBoneIndex(
            *m_context.playerModel,
            "Gun.R",
            m_context.references.gunRBoneIndex);
    }

    m_context.cameraTransform = context.mainCameraTransform;
    m_context.cameraComponent = context.mainCamera;
    m_context.cameraControlBehavior = context.cameraControlBehavior;

    m_context.animationController = context.animationController;
    m_context.locomotionController = context.locomotionController;
    m_context.weaponController = context.weaponController;

    m_context.aim.Initialize(m_context);
    m_context.firing.Initialize(m_context);
    m_context.effects.Initialize(m_context);
    m_context.rapidFire.Initialize(m_context);
    m_context.slashBurst.Initialize(m_context);
}

bool PlayerDualPistolsAction::CanStart(const PlayerContext& context, const PlayerInput& input)
{
    // エイム入力が無いかつ、攻撃入力がある場合開始できる
    return !(input.triggerAimCommand || input.holdAimCommand) && (input.triggerAttackCommand || input.holdAttackCommand);
}

bool PlayerDualPistolsAction::IsReleaseAttackInput(const PlayerInput& input) const
{
    return !input.holdAttackCommand;
}

void PlayerDualPistolsAction::Start(PlayerContext& context, const PlayerInput& input)
{
    // アニメーションのサブマシーンを切り替える（切替に失敗した場合は、アクションを終了する）
    m_enteredAnimationSubMachine = m_context.animationController->EnterSubMachine(
        PlayerAnimationController::SubMachine::DualPistols);
    if (!m_enteredAnimationSubMachine) {
        SetState(ActionState::WaitingToFinish);
        return;
    }

    m_context.runtimeState.phase = PlayerDualPistolsRuntimeState::Phase::Entering;
    m_context.runtimeState.phaseTimer = 0.0f;

    m_enteredPhase = true;
}

void PlayerDualPistolsAction::Update(PlayerContext& context, const PlayerInput& input, float deltaTime)
{
    Phase currentPhase = m_context.runtimeState.phase;
    bool enteredPhase = m_enteredPhase;
    m_enteredPhase = false;

    m_context.runtimeState.phaseTimer += deltaTime;
    m_context.runtimeState.releaseAttackInput = IsReleaseAttackInput(input);

    switch (currentPhase) {
        // === 単押しか長押しかの入力解決を行う ===
        case Phase::Entering: {
            if (m_context.runtimeState.releaseAttackInput) {
                ChangePhase(Phase::SlashBurst);
            }
            else if (m_context.runtimeState.phaseTimer >= m_context.settings().startRapidFireDelay) {
                ChangePhase(Phase::RapidFire);
            }
            break;
        }
        case Phase::SlashBurst: {
            if (enteredPhase) {
                m_context.slashBurst.Start(m_context);
            }

            m_context.slashBurst.Update(m_context, deltaTime);

            if (m_context.slashBurst.IsFinished() || !m_context.slashBurst.IsActive()) {
                ChangePhase(Phase::Exiting);
            }
            break;
        }
        case Phase::RapidFire: {
            if (enteredPhase) {
                m_context.rapidFire.Start(m_context);
            }

            m_context.rapidFire.Update(m_context, deltaTime);

            if (m_context.runtimeState.releaseAttackInput || (input.triggerAimCommand || input.holdAimCommand)) {
                m_context.rapidFire.Stop(m_context);
                ChangePhase(Phase::Exiting);
            }
            break;
        }
        case Phase::Exiting: {
            SetState(ActionState::WaitingToFinish);
            break;
        }
        default: break;
    }

}

void PlayerDualPistolsAction::Finish(PlayerContext& context, const PlayerInput& input)
{
    m_context.rapidFire.Reset(m_context);
    m_context.slashBurst.Cancel(m_context);
    m_context.rapidFire.Stop(m_context);

    m_context.runtimeState = {};
    m_enteredPhase = false;
    if (m_enteredAnimationSubMachine) {
        m_context.animationController->RequestExitSubMachine();
        m_enteredAnimationSubMachine = false;
    }
}

void PlayerDualPistolsAction::ChangePhase(PlayerDualPistolsRuntimeState::Phase newPhase)
{
    if (m_context.runtimeState.phase == newPhase) {
        return;
    }
    m_context.runtimeState.phase = newPhase;
    m_context.runtimeState.phaseTimer = 0.0f;
    m_enteredPhase = true;
}
