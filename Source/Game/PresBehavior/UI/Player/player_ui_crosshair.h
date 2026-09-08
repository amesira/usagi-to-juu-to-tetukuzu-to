#pragma once
#include "player_ui_widget_element.h"
#include <array>

class PlayerUiCrosshair {
public:
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group);
    void ApplyLayout();
    void Destroy();
private:
    PlayerUi::WidgetGroup* m_group = nullptr;
    std::array<PlayerUiWidgetElement, 4> m_lines;
};
