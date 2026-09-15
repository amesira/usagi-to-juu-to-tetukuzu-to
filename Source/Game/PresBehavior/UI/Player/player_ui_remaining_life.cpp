//===================================================
// File  ：_/UI/Player/player_ui_remaining_life.cpp
// Date  ：2026/09/08
// Author：Miu Kitamura
//===================================================
#include "player_ui_remaining_life.h"
#include "Game/Factory/ui_factory.h"
#include "Engine/Component/image_component.h"
#include "External/ImGui/imgui.h"
#include <algorithm>

void PlayerUiRemainingLife::Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
    const PlayerUiSettings::RemainingLifeSettings& settings)
{
    if (m_group || !scene) return;
    m_group = &group;
    m_group->applyPerspective = false;

    m_gauge.Register(group, UiFactory::CreateUiImageHandle(scene,
        L"asset/Texture/Ui/player_remaining_life_gauge.png"), "PlayerUi.RemainingLife", 100);
    if (auto* image = m_gauge.handle.GetImage()) {
        image->SetFillMethod(ImageComponent::FillMethod::Horizontal);
    }
    ApplyLayout(settings);
    UpdateDisplay();
}

void PlayerUiRemainingLife::ApplyLayout(const PlayerUiSettings::RemainingLifeSettings& settings)
{
    if (m_group) {
        m_gauge.ApplyLayout(*m_group, settings.gauge);
    }
}

void PlayerUiRemainingLife::SetRemainingLife(int current, int maximum)
{
    m_fillAmount = maximum > 0 ? std::clamp(static_cast<float>(current) / maximum, 0.0f, 1.0f) : 0;
    UpdateDisplay();
}

void PlayerUiRemainingLife::UpdateDisplay()
{
    if (!m_group) return;
    if (auto* image = m_gauge.handle.GetImage()) image->SetFillAmount(m_fillAmount);
}

void PlayerUiRemainingLife::DrawInspector()
{
    ImGui::PushID(this);
    if (ImGui::SliderFloat("Remaining Life Fill", &m_fillAmount, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp))
        UpdateDisplay();
    ImGui::PopID();
}

void PlayerUiRemainingLife::Destroy()
{
    if (!m_group) return;
    for (auto& widget : m_group->widgets) widget.Destroy();
    *m_group = {};
    m_group = nullptr;
    m_gauge = {};
}

void PlayerUiRemainingLife::ApplyColors(const DirectX::XMFLOAT3& color2)
{
    if (!m_group) return;
    for (UiHandle& widget : m_group->widgets) {
        widget.SetColor(color2);
    }
}
