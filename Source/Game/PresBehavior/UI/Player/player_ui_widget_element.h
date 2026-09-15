// player_ui_widget_element.h
// 2026/09/08
// ・UiHandleを内包した構造体で、ウィジェットの登録とレイアウト適用を行う
#pragma once
#include "player_ui_context.h"
#include "player_ui_settings_asset.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/slider_component.h"

// ウィジェットの登録とレイアウト適用を行う構造体
struct PlayerUiWidgetElement {
    UiHandle handle;
    size_t slot = 0;

    void Register(PlayerUi::WidgetGroup& group, UiHandle value, const char* name, float layer) {
        handle = value;
        slot = group.widgets.size();
        group.widgets.push_back(value);
        group.offsetPositions.push_back({});
        if (auto* object = handle.GetGameObject()) object->SetName(name);
        if (auto* rect = handle.GetRectTransform()) rect->SetPosition({0, 0, layer});
    }
    void ApplyLayout(PlayerUi::WidgetGroup& group, const PlayerUiSettings::WidgetTransform& layout) {
        group.offsetPositions[slot] = layout.position;
        handle.SetSize(layout.size.x, layout.size.y);
        handle.SetPosition(group.currentCenterPosition.x + layout.position.x,
            group.currentCenterPosition.y + layout.position.y);
        if (auto* rect = handle.GetRectTransform())
            rect->SetRotation({0, 0, DirectX::XMConvertToRadians(layout.rotationDegrees)});
    }
};