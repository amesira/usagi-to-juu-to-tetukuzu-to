//===================================================
// File  ：_/UI/Player/player_ui_view.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_view.h"
#include <algorithm>
#include <cmath>

void PlayerUiView::SetWidgetGroupAlpha(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, float alpha)
{
    auto& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.currentAlpha = std::clamp(alpha, 0.0f, 1.0f);
    for (auto& widget : group.widgets) {
        widget.SetAlpha(group.currentAlpha);
    }
}

void PlayerUiView::SetWidgetGroupPosition(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, const DirectX::XMFLOAT2& position)
{
    auto& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.originalCenterPosition = position;
    ApplyWidgetGroupPosition(group);
}

void PlayerUiView::SetWidgetGroupShakeOffset(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, const DirectX::XMFLOAT2& offset)
{
    auto& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.shakeOffset = offset;
    ApplyWidgetGroupPosition(group);
}

/// @brief ウィジェットグループの位置を演出オフセット込みで反映する
void PlayerUiView::ApplyWidgetGroupPosition(PlayerUi::WidgetGroup& group)
{
    group.currentCenterPosition = {
        group.originalCenterPosition.x + group.shakeOffset.x,
        group.originalCenterPosition.y + group.shakeOffset.y };

    // 配置未登録のウィジェットは移動させない
    const size_t count = (std::min)(group.widgets.size(), group.offsetPositions.size());
    for (size_t i = 0; i < count; ++i) {
        group.widgets[i].SetPosition(
            group.currentCenterPosition.x + group.offsetPositions[i].x,
            group.currentCenterPosition.y + group.offsetPositions[i].y);
    }
}