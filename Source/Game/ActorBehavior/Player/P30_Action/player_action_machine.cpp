//===================================================
// File  ：_/Player/P30_Action/player_action_machine.cpp
// Date  ：2026/08/20
// Author：Miu Kitamura
// 
// ・Playerの行動を管理するマシン
//===================================================
#include "player_action_machine.h"

// engine
#include "Engine/Device/mi_fps.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

/// @brief 初期化処理
void PlayerActionMachine::Initialize(const PlayerContext& context, const PlayerInput& input)
{
    // 各Actionの初期化

}

/// @brief 更新処理
void PlayerActionMachine::Update(PlayerContext& context, const PlayerInput& input)
{
    float deltaTime = FPS_GetDeltaTime();

    // === バックグラウンド更新 ===
    for (auto* action : m_allActions) {
        action->UpdateBackground(context, input, deltaTime);
    }

    // === 現在の行動の更新 ===
    if (m_currentAction) {
        m_currentAction->Update(context, input, deltaTime);

        // 行動が終了待機状態になった場合、Finishを呼び出す
        if (m_currentAction->GetState() == PlayerActionBase::ActionState::WaitingToFinish) {
            m_currentAction->Finish(context, input);
            m_currentAction = nullptr; // 現在の行動をクリア
        }
    }

    bool canStartNewAction = true;
    if (m_currentAction) {
        // 現在の行動が割り込み可能でない場合、新しい行動を開始できない
        canStartNewAction = m_currentAction->IsInterruptible();
    }

    if (canStartNewAction){
        // === 新しい行動の開始 ===
        PlayerActionBase* newActionToStart = nullptr;
        for (auto* action : m_allActions) {
            if (action->CanStart(context, input)) {
                // 優先度が高い行動を選択
                if (!newActionToStart || action->GetPriority() > newActionToStart->GetPriority()) {
                    newActionToStart = action;
                }
            }
        }

        if (newActionToStart) {
            // 現在の行動を終了させる
            if (m_currentAction) {
                m_currentAction->Finish(context, input);
            }
            // 新しい行動を開始する
            newActionToStart->Start(context, input);
            m_currentAction = newActionToStart;
        }
    }
}