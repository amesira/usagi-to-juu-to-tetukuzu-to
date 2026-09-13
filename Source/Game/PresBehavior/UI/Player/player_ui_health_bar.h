//---------------------------------------------------
// File  ：_/UI/Player/player_ui_health_bar.h
// Date  ：2026/09/08
// Author：Miu Kitamura
// 
// ・PlayerUiのHPバー表示を担当するクラス
//---------------------------------------------------
#pragma once
#include "player_ui_widget_element.h"
#include <array>

class PlayerUiHealthBar {
private:
    PlayerUi::WidgetGroup* m_group = nullptr;
    PlayerUiWidgetElement m_slider;
    PlayerUiWidgetElement m_label;
    PlayerUiWidgetElement m_recoveryGauge;
    std::array<PlayerUiWidgetElement, 3> m_markers;

    float m_health = 100;
    float m_maxHealth = 100;
    float m_recovery = 0.0f;

public:
    void ApplyColors(const DirectX::XMFLOAT3& color1, const DirectX::XMFLOAT3& color2);
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
        const PlayerUiSettings::HealthBarSettings& settings);
    void ApplyLayout(const PlayerUiSettings::HealthBarSettings& settings);
    void UpdateMarkers();

    void DrawInspector();
    void Destroy();

    void SetHealth(float current, float maximum);
    void SetRecovery(float amount);

private:
    void UpdateDisplay();
    PlayerUiSettings::WidgetTransform MakeHealthMarker(const PlayerUiSettings::WidgetTransform& bar, float fraction);
    
};
