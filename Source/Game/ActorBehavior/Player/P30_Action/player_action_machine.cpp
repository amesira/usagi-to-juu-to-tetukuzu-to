//===================================================
// File  ：_/Player/P30_Action/player_action_machine.cpp
// Date  ：2026/08/20
// Author：Miu Kitamura
// 
// ・Playerの行動を管理するマシン
//===================================================
#include "player_action_machine.h"
#include <algorithm>

// engine
#include "Engine/Device/mi_fps.h"

#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

/// @brief 初期化処理
void PlayerActionMachine::Initialize(const PlayerContext& context, const PlayerInput& input)
{
    m_allActions.clear();
    m_activeActions.clear();

    m_categoryRestrictions.clear();
    m_categoryRestrictions[ActionCategory::Attack] = ActionRestriction{
        ActionCategoryMask(0), // カテゴリごとの禁止アクションはなし
        {}
    };
    m_categoryRestrictions[ActionCategory::Interaction] = ActionRestriction{
        ActionCategoryMask(ActionCategory::Attack),
        {}
    };
    m_categoryRestrictions[ActionCategory::Reaction] = ActionRestriction{
        ActionCategoryMask(PlayerActionBase::ToMask(ActionCategory::Attack) | PlayerActionBase::ToMask(ActionCategory::Interaction)),
        {"Dodge", "Jet"}
    };
}

/// @brief PlayerBehaviorが所有するActionを登録する
void PlayerActionMachine::RegisterAction(PlayerActionBase& action)
{
    if (std::find(m_allActions.begin(), m_allActions.end(), &action) != m_allActions.end()) {
        return;
    }

    m_allActions.push_back(&action);
}

/// @brief 更新処理
void PlayerActionMachine::Update(PlayerContext& context, const PlayerInput& input)
{
    if (m_allActions.empty()) return;
    float deltaTime = FPS_GetDeltaTime();

    // === バックグラウンド更新 ===
    for (auto& action : m_allActions) {
        action->UpdateBackground(context, input, deltaTime);
    }

    // === 行動終了処理 ===
    for (auto& action : m_activeActions) {
        if (!action) continue;
        // 行動が終了待機状態になった場合、Finishを呼び出す
        // FIX: 何が由来の終了待機なのかをリクエストにした方が良いかも（通常か、キャンセルか、割り込みかなど）
        if (action->GetState() == PlayerActionBase::ActionState::WaitingToFinish) {
            action->Finish(context, input);
            action->SetState(PlayerActionBase::ActionState::None);
            action = nullptr;
        }
    }
    m_activeActions.erase(std::remove(m_activeActions.begin(), m_activeActions.end(), nullptr), m_activeActions.end());

    // === アクティブな行動の更新 ===
    for (auto& action : m_activeActions) {
        if (!action) continue;
        action->Update(context, input, deltaTime);
    }

    // === 新規アクションの開始 ===
    std::vector<PlayerActionBase*> newActiveActions;
    for (auto& action : m_allActions) {
        if (!action->CanStart(context, input)) continue;
        if (std::find(m_activeActions.begin(), m_activeActions.end(), action) != m_activeActions.end()) continue;
        newActiveActions.push_back(action);
    }
    // 1フレーム内に開始と割り込み終了が起こるのを防ぎたいので、優先度の高い順にソートしてから開始する
    std::sort(newActiveActions.begin(), newActiveActions.end(), [](PlayerActionBase* a, PlayerActionBase* b) {
        return a->GetPriority() > b->GetPriority();
        });
    for (auto& newAction : newActiveActions) {
        if (EvaluateStart(newAction)) {
            newAction->SetState(PlayerActionBase::ActionState::Active);
            newAction->Start(context, input);
            m_activeActions.push_back(newAction);
        }
    }

    // === 制約の解決 ===
    ResolveRestrictions();
}

/// @brief 指定されたアクションを開始できるかどうかを判定する
bool PlayerActionMachine::EvaluateStart(PlayerActionBase* action)
{
    std::vector<PlayerActionBase*> requestFinishActions;

    for (auto& activeAction : m_activeActions) {
        if (!activeAction) continue;

        // アクションのカテゴリが同じ場合
        const bool isSameExclusiveCategory =
            action->GetCategory() != ActionCategory::None &&
            activeAction->GetCategory() == action->GetCategory();
        if (isSameExclusiveCategory) {
            if (!activeAction->IsInterruptible()) {
                return false;
            }
            if (activeAction->GetPriority() >= action->GetPriority()) {
                return false;
            }

            // 現在のアクションを終了待機状態にする
            requestFinishActions.push_back(activeAction);
        }

        // 禁止アクションに含まれている場合
        auto categoryIt = m_categoryRestrictions.find(activeAction->GetCategory());
        if (categoryIt != m_categoryRestrictions.end()) {
            const auto& restriction = categoryIt->second;
            if ((restriction.blockedCategories & PlayerActionBase::ToMask(action->GetCategory())) != 0) {
                return false;
            }
            if (std::find(restriction.blockedActionIDs.begin(), restriction.blockedActionIDs.end(), action->GetActionID()) != restriction.blockedActionIDs.end()) {
                return false;
            }
        }
        auto actionIt = m_actionRestrictions.find(activeAction->GetActionID());
        if (actionIt != m_actionRestrictions.end()) {
            const auto& restriction = actionIt->second;
            if ((restriction.blockedCategories & PlayerActionBase::ToMask(action->GetCategory())) != 0) {
                return false;
            }
            if (std::find(restriction.blockedActionIDs.begin(), restriction.blockedActionIDs.end(), action->GetActionID()) != restriction.blockedActionIDs.end()) {
                return false;
            }
        }
    }

    for (auto& activeAction : requestFinishActions) {
        activeAction->SetState(PlayerActionBase::ActionState::WaitingToFinish);
    }
    return true;
}

/// @brief 現在のアクションの制約を解決する
void PlayerActionMachine::ResolveRestrictions()
{
    for (auto& activeAction : m_activeActions) {
        if (!activeAction) continue;
        // カテゴリごとの禁止制約を確認
        auto categoryIt = m_categoryRestrictions.find(activeAction->GetCategory());
        if (categoryIt != m_categoryRestrictions.end()) {
            const auto& restriction = categoryIt->second;

            for (auto& otherAction : m_activeActions) {
                if (!otherAction || otherAction == activeAction) continue;
                // カテゴリの禁止制約を確認
                if ((restriction.blockedCategories & PlayerActionBase::ToMask(otherAction->GetCategory())) != 0) {
                    otherAction->SetState(PlayerActionBase::ActionState::WaitingToFinish);
                }
                // アクションIDの禁止制約を確認
                if (std::find(restriction.blockedActionIDs.begin(), restriction.blockedActionIDs.end(), otherAction->GetActionID()) != restriction.blockedActionIDs.end()) {
                    otherAction->SetState(PlayerActionBase::ActionState::WaitingToFinish);
                }
            }
        }
        // アクションIDごとの禁止制約を確認
        auto actionIt = m_actionRestrictions.find(activeAction->GetActionID());
        if (actionIt != m_actionRestrictions.end()) {
            const auto& restriction = actionIt->second;
            for (auto& otherAction : m_activeActions) {
                if (!otherAction || otherAction == activeAction) continue;
                // カテゴリの禁止制約を確認
                if ((restriction.blockedCategories & PlayerActionBase::ToMask(otherAction->GetCategory())) != 0) {
                    otherAction->SetState(PlayerActionBase::ActionState::WaitingToFinish);
                }
                // アクションIDの禁止制約を確認
                if (std::find(restriction.blockedActionIDs.begin(), restriction.blockedActionIDs.end(), otherAction->GetActionID()) != restriction.blockedActionIDs.end()) {
                    otherAction->SetState(PlayerActionBase::ActionState::WaitingToFinish);
                }
            }
        }
    }
}
