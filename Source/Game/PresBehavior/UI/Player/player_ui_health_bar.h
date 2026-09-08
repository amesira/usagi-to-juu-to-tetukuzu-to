#pragma once
#include "player_ui_widget_element.h"
#include <array>

class PlayerUiHealthBar {
public:
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
        const PlayerUiSettings::HealthBarSettings& settings);
    void ApplyLayout(const PlayerUiSettings::HealthBarSettings& settings);
    void UpdateMarkers();
    void DrawInspector();
    // Explicit destruction; the scene owns objects during whole-scene teardown.
    void Destroy();
    void SetHealth(float current, float maximum);
    void SetRecovery(float amount);
private:
    void UpdateDisplay();
    PlayerUi::WidgetGroup* m_group = nullptr;
    PlayerUiWidgetElement m_slider, m_label, m_recoveryGauge;
    std::array<PlayerUiWidgetElement, 3> m_markers;
    float m_health = 100, m_maxHealth = 100, m_recovery = 1;
};
