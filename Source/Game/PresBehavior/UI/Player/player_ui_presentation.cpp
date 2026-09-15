//===================================================
// File  ：_/UI/Player/player_ui_presentation.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_presentation.h"

#include "player_ui_view.h"

#include <algorithm>
#include <cmath>

using namespace PlayerUi;

void PlayerUiPresentation::Initialize()
{
    m_groupTasks = {};
}

void PlayerUiPresentation::Update(PlayerUiContext& context, float deltaTime)
{
    if (!context.view || !std::isfinite(deltaTime) || deltaTime < 0.0f) return;

    for (size_t i = 1; i < m_groupTasks.size(); ++i) {
        const auto id = static_cast<PlayerUi::WidgetGroupID>(i);
        auto& tasks = m_groupTasks[i];
        if (!tasks.fade.IsFinished()) {
            tasks.fade.Update(deltaTime);
            // 終了したフレームも目標値を反映する。
            context.view->SetWidgetGroupAlpha(context, id, tasks.fade.m_currentValue);
        }
        if (!tasks.shake.IsFinished()) {
            tasks.shake.Update(deltaTime);
            context.view->SetWidgetGroupShakeOffset(context, id, tasks.shake.m_currentOffset);
        }
    }
}

/// @brief ウィジェットグループのアルファ値をフェードさせる
void PlayerUiPresentation::FadeWidgetGroup(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, float targetAlpha, float duration)
{
    if (!context.view) return;

    const size_t index = static_cast<size_t>(groupID);
    auto& task = m_groupTasks[index].fade;
    task.Reset();
    targetAlpha = std::clamp(targetAlpha, 0.0f, 1.0f);
    if (duration <= 0.0f) {
        context.view->SetWidgetGroupAlpha(context, groupID, targetAlpha);
        return;
    }

    task.m_startValue = context.widgetGroups[index].currentAlpha;
    task.m_targetValue = targetAlpha;
    task.m_endValue = targetAlpha;
    task.m_duration = duration;
    task.m_holdDuration = 0.0f;
    task.Start();
}

/// @brief ウィジェットグループを揺らす演出を開始する
void PlayerUiPresentation::ShakeWidgetGroup(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, float intensity, float duration)
{
    if (!context.view) return;

    auto& task = m_groupTasks[static_cast<size_t>(groupID)].shake;
    task.Cancel();
    context.view->SetWidgetGroupShakeOffset(context, groupID, {});
    task.m_intensity = intensity;
    task.m_duration = duration;
    task.Start();
}

#pragma region Cancel演出
void PlayerUiPresentation::CancelFadeWidgetGroup(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID)
{
    auto& task = m_groupTasks[static_cast<size_t>(groupID)].fade;
    task.Reset();
    task.m_currentValue = context.widgetGroups[static_cast<size_t>(groupID)].currentAlpha;
}

void PlayerUiPresentation::CancelShakeWidgetGroup(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID)
{
    m_groupTasks[static_cast<size_t>(groupID)].shake.Cancel();
    if (context.view) {
        context.view->SetWidgetGroupShakeOffset(context, groupID, {});
    }
    else {
        auto& group = context.widgetGroups[static_cast<size_t>(groupID)];
        group.shakeOffset = {};
        group.currentCenterPosition = group.originalCenterPosition;
    }
}

void PlayerUiPresentation::CancelAll(PlayerUiContext& context)
{
    for (size_t i = 1; i < m_groupTasks.size(); ++i) {
        const auto id = static_cast<PlayerUi::WidgetGroupID>(i);
        CancelFadeWidgetGroup(context, id);
        CancelShakeWidgetGroup(context, id);
    }
}
#pragma endregion