//===================================================
// File  ：_/UI/Player/player_ui_crosshair.cpp
// Date  ：2026/09/08
// Author：Miu Kitamura
//===================================================
#include "player_ui_crosshair.h"
#include "Game/Factory/ui_factory.h"

void PlayerUiCrosshair::Initialize(IScene* scene, PlayerUi::WidgetGroup& group)
{
    if (m_group || !scene) return;

    m_group = &group;
    m_group->applyPerspective = false; // Crosshairは奥行き変換を無効化する

    const char* names[] = {
        "PlayerUi.Crosshair.Left.Test", "PlayerUi.Crosshair.Right.Test",
        "PlayerUi.Crosshair.Top.Test", "PlayerUi.Crosshair.Bottom.Test"
    };

    for (size_t i = 0; i < m_lines.size(); ++i) {
        m_lines[i].Register(group, UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp"), names[i], 102);
    }
    ApplyLayout();
}

void PlayerUiCrosshair::ApplyLayout()
{
    if (!m_group) return;

    const DirectX::XMFLOAT2 offsets[] = {{-10, 0}, {10, 0}, {0, -10}, {0, 10}};
    const DirectX::XMFLOAT2 sizes[] = {{8, 2}, {8, 2}, {2, 8}, {2, 8}};

    for (size_t i = 0; i < m_lines.size(); ++i) {
        m_lines[i].ApplyLayout(*m_group, {offsets[i], sizes[i], 0});
    }
}

void PlayerUiCrosshair::Destroy()
{
    if (!m_group) return;

    for (auto& widget : m_group->widgets) {
        widget.Destroy();
    }

    *m_group = {};
    m_group = nullptr;
    m_lines = {};
}

void PlayerUiCrosshair::ApplyColors(const DirectX::XMFLOAT3& color2)
{
    if (!m_group) return;

    for (auto& widget : m_group->widgets) {
        widget.SetColor(color2);
    }
}
