//---------------------------------------------------
// File  ：_/UI/Player/player_ui_ammo_count.h
// Date  ：2026/09/08
// Author：Miu Kitamura
// 
// ・PlayerUiの弾薬数表示を担当するクラス
//---------------------------------------------------
#pragma once
#include "player_ui_widget_element.h"
#include <array>

class PlayerUiAmmoCount {
private:
    PlayerUi::WidgetGroup* m_group = nullptr;
    PlayerUiWidgetElement m_background;
    PlayerUiWidgetElement m_fill;
    PlayerUiWidgetElement m_icon;
    PlayerUiWidgetElement m_currentText;
    PlayerUiWidgetElement m_capacityText;
    std::array<PlayerUiWidgetElement, 3> m_markers;
    int m_ammoCount = 0;
    int m_ammoCapacity = 0;

public:
    void ApplyColors(const DirectX::XMFLOAT3& color1, const DirectX::XMFLOAT3& color2);
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
        const PlayerUiSettings::AmmoCountSettings& settings);
    void ApplyLayout(const PlayerUiSettings::AmmoCountSettings& settings);
    void UpdateMarkers();

    void DrawInspector();
    void Destroy();
    
    void SetAmmoCount(int current, int maximum);

private:
    void UpdateDisplay();

};
