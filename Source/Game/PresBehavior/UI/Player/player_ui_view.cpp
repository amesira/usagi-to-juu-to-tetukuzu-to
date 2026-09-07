//===================================================
// File  ：_/UI/Player/player_ui_view.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_view.h"

using namespace PlayerUi;

/// @brief ウィジェットグループのアルファ値を設定する
void PlayerUiView::SetWidgetGroupAlpha(
    PlayerUiContext& context, 
    PlayerUi::WidgetGroupID groupID, 
    float alpha)
{
    WidgetGroup& group = context.widgetGroups[static_cast<size_t>(groupID)];
    for (UiHandle& widget : group.widgets) {
        widget.SetAlpha(alpha);
    }
}

/// @brief ウィジェットグループの位置を設定する
void PlayerUiView::SetWidgetGroupPosition(
    PlayerUiContext& context, 
    PlayerUi::WidgetGroupID groupID, 
    const DirectX::XMFLOAT2& position)
{
    WidgetGroup& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.currentCenterPosition = position;

    for (size_t i = 0; i < group.widgets.size(); ++i) {
        UiHandle& widget = group.widgets[i];
        widget.SetPosition(
            position.x + group.offsetPositions[i].x,
            position.y + group.offsetPositions[i].y
        );
    }
}
