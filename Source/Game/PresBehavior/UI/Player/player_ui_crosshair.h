//---------------------------------------------------
// File  ：_/UI/Player/player_ui_crosshair.h
// Date  ：2026/09/08
// Author：Miu Kitamura
// 
// ・PlayerUiの十字線表示を担当するクラス
//---------------------------------------------------
#pragma once
#include "player_ui_widget_element.h"
#include <array>

class PlayerUiCrosshair {
private:
    PlayerUi::WidgetGroup* m_group = nullptr;
    std::array<PlayerUiWidgetElement, 4> m_lines;

public:
    void ApplyColors(const DirectX::XMFLOAT3& color2);
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group);
    void ApplyLayout();
    
    void Destroy();

};
