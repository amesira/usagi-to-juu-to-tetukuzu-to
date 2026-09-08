#pragma once
#include "player_ui_widget_element.h"
#include <array>

class PlayerUiAmmoCount {
public:
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
        const PlayerUiSettings::AmmoCountSettings& settings);
    void ApplyLayout(const PlayerUiSettings::AmmoCountSettings& settings);
    void UpdateMarkers();
    void DrawInspector();
    // Explicit destruction; the scene owns objects during whole-scene teardown.
    void Destroy();
    void SetAmmoCount(int current);
private:
    void UpdateDisplay();
    PlayerUi::WidgetGroup* m_group = nullptr;
    PlayerUiWidgetElement m_background, m_fill, m_icon, m_currentText, m_capacityText;
    std::array<PlayerUiWidgetElement, 3> m_markers;
    int m_ammoCount = 20;
};
