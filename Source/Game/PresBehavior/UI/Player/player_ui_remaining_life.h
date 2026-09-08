#pragma once
#include "player_ui_widget_element.h"

class PlayerUiRemainingLife {
public:
    void ApplyColors(const DirectX::XMFLOAT3& color2);
    void Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
        const PlayerUiSettings::RemainingLifeSettings& settings);
    void ApplyLayout(const PlayerUiSettings::RemainingLifeSettings& settings);
    void SetRemainingLife(int current, int maximum);
    void DrawInspector();
    void Destroy();
private:
    void UpdateDisplay();
    PlayerUi::WidgetGroup* m_group = nullptr;
    PlayerUiWidgetElement m_gauge;
    float m_fillAmount = 1;
};
