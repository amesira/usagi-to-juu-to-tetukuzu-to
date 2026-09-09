//===================================================
// File  ：_/UI/Player/player_ui_view.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_view.h"

#include <algorithm>
#include <cmath>

#include "player_ui_settings_asset.h"
#include "player_ui_perspective.h"

#include "Engine/Component/rect_transform_component.h"

using namespace PlayerUi;

void PlayerUiView::SetWidgetGroupAlpha(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, float alpha)
{
    WidgetGroup& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.currentAlpha = std::clamp(alpha, 0.0f, 1.0f);
    for (UiHandle& widget : group.widgets) {
        widget.SetAlpha(group.currentAlpha);
    }
}

void PlayerUiView::SetWidgetGroupPosition(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, const DirectX::XMFLOAT2& position)
{
    WidgetGroup& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.originalCenterPosition = position;
    ApplyWidgetGroupPosition(context, group);
}

/// @brief ウィジェットグループの位置を基準位置を変えずに演出オフセットだけを反映する
void PlayerUiView::SetWidgetGroupShakeOffset(PlayerUiContext& context,
    PlayerUi::WidgetGroupID groupID, const DirectX::XMFLOAT2& offset)
{
    WidgetGroup& group = context.widgetGroups[static_cast<size_t>(groupID)];
    group.shakeOffset = offset;
    ApplyWidgetGroupPosition(context, group);
}

/// @brief ウィジェットグループの位置を演出オフセット込みで反映する
void PlayerUiView::ApplyWidgetGroupPosition(PlayerUiContext& context, PlayerUi::WidgetGroup& group)
{
    group.currentCenterPosition = {
        group.originalCenterPosition.x + group.shakeOffset.x,
        group.originalCenterPosition.y + group.shakeOffset.y };

    static const PlayerUiSettings::Data defaults;
    const auto& settings = context.settingsAsset ? context.settingsAsset->GetData() : defaults;

    PlayerUiSettings::PerspectiveSettings perspective = context.runtimeState.runningPerspective;
    if (!group.applyPerspective) { // Perspectiveを適用しないグループは無効化する
        perspective.enabled = false;
    }

    // 奥行き変換行列を作成
    const auto transform = PlayerUiPerspective::MakeTransform(
        perspective,
        group.originalCenterPosition, 
        group.currentCenterPosition,
        context.runtimeState.screenSize);
    
    DirectX::XMFLOAT2 screenAnchor = {0.5f, 0.5f};
    switch (static_cast<WidgetGroupID>(&group - context.widgetGroups)) {
        case WidgetGroupID::HealthBar:
            screenAnchor = settings.healthBar.placement.screenAnchor;
            break;
        case WidgetGroupID::AmmoCount:
            screenAnchor = settings.ammoCount.placement.screenAnchor;
            break;
        case WidgetGroupID::RemainingLife:
            screenAnchor = settings.remainingLife.placement.screenAnchor;
            break;
        default: break;
    }

    const UiChromaticEcho echo = PlayerUiSettings::ResolveChromaticEcho(
        settings.chromaticEcho,
        screenAnchor,
        context.runtimeState.screenSize);

    // === 適用 ===
    // 配置未登録のウィジェットは移動させない
    const size_t count = (std::min)(group.widgets.size(), group.offsetPositions.size());
    for (size_t i = 0; i < count; ++i) {
        if (auto* rect = group.widgets[i].GetRectTransform()) {
            rect->SetPresentationTransform(transform);
            rect->SetChromaticEcho(echo);
        }
        group.widgets[i].SetPosition(
            group.currentCenterPosition.x + group.offsetPositions[i].x,
            group.currentCenterPosition.y + group.offsetPositions[i].y);
    }
}
