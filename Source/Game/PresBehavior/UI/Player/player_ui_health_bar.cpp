#include "player_ui_health_bar.h"
#include "Game/Factory/ui_factory.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/slider_component.h"
#include "External/ImGui/imgui.h"
#include <algorithm>
#include <cmath>
#include <string>
using namespace DirectX;
void PlayerUiHealthBar::Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
    const PlayerUiSettings::HealthBarSettings& settings)
{
    if (m_group || !scene) return;
    m_group = &group;
    m_slider.Register(group, UiFactory::CreateUiSliderHandle(scene,
        {0.08f, 0.1f, 0.14f, 1}, {0.2f, 0.9f, 0.4f, 1}, 1), "PlayerUi.HealthBar", 100);
    m_label.Register(group, UiFactory::CreateUiTextHandle(scene, u8""), "PlayerUi.HealthText", 104);
    if (auto* text = m_label.handle.GetText()) {
        text->SetFontSize(22);
        text->SetColor({1, 1, 1, 1});
        text->SetCenter(false);
    }
    m_recoveryGauge.Register(group, UiFactory::CreateUiImageHandle(scene,
        L"asset/Texture/Ui/recovery_gauge.png"), "PlayerUi.RecoveryGauge", 100);
    if (auto* image = m_recoveryGauge.handle.GetImage()) image->SetFillMethod(ImageComponent::FillMethod::Horizontal);
    const char* names[] = {"PlayerUi.HealthLeft", "PlayerUi.HealthRight", "PlayerUi.HealthFill"};
    for (size_t i = 0; i < m_markers.size(); ++i)
        m_markers[i].Register(group, UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp"), names[i], 103);
    ApplyLayout(settings);
    UpdateDisplay();
}
void PlayerUiHealthBar::ApplyLayout(const PlayerUiSettings::HealthBarSettings& settings)
{
    if (!m_group) return;
    m_slider.ApplyLayout(*m_group, settings.slider);
    m_label.ApplyLayout(*m_group, settings.label);
    m_recoveryGauge.ApplyLayout(*m_group, settings.recoveryGauge);
    UpdateMarkers();
}
void PlayerUiHealthBar::SetHealth(float current, float maximum)
{
    m_maxHealth = std::isfinite(maximum) ? (std::max)(maximum, 0.0f) : 0;
    m_health = std::isfinite(current) ? std::clamp(current, 0.0f, m_maxHealth) : 0;
    UpdateDisplay();
}
void PlayerUiHealthBar::SetRecovery(float amount)
{
    m_recovery = std::isfinite(amount) ? std::clamp(amount, 0.0f, 1.0f) : 0;
    UpdateDisplay();
}
void PlayerUiHealthBar::UpdateDisplay()
{
    if (!m_group) return;
    if (auto* slider = m_slider.handle.GetSlider()) slider->SetValue(m_maxHealth > 0 ? m_health / m_maxHealth : 0);
    if (auto* text = m_label.handle.GetText()) text->SetText(
        std::to_string(static_cast<int>(std::ceil(m_health))) + " / " + std::to_string(static_cast<int>(std::ceil(m_maxHealth))));
    if (auto* image = m_recoveryGauge.handle.GetImage()) image->SetFillAmount(m_recovery);
    UpdateMarkers();
}
void PlayerUiHealthBar::UpdateMarkers()
{
    if (!m_group) return;
    auto* rect = m_slider.handle.GetRectTransform();
    auto* slider = m_slider.handle.GetSlider();
    if (!rect || !slider) return;
    const auto size = rect->GetScaling();
    const PlayerUiSettings::WidgetTransform bar = {m_group->offsetPositions[m_slider.slot],
        {size.x, size.y}, XMConvertToDegrees(rect->GetRotation().z)};
    const float fractions[] = {0, 1, slider->GetValue()};
    for (size_t i = 0; i < m_markers.size(); ++i) {
        m_markers[i].ApplyLayout(*m_group, PlayerUiSettings::MakeHealthMarker(bar, fractions[i]));
        if (auto* marker = m_markers[i].handle.GetRectTransform())
            marker->SetPresentationTransform(rect->GetPresentationTransform());
    }
}
void PlayerUiHealthBar::DrawInspector()
{
    ImGui::PushID(this);
    float maximum = m_maxHealth;
    if (ImGui::DragFloat("Max HP", &maximum, 1, 0, 100000, "%.0f", ImGuiSliderFlags_AlwaysClamp))
        SetHealth(m_health, maximum);
    float health = m_health;
    if (ImGui::SliderFloat("HP", &health, 0, m_maxHealth, "%.0f", ImGuiSliderFlags_AlwaysClamp))
        SetHealth(health, m_maxHealth);
    float recovery = m_recovery;
    if (ImGui::SliderFloat("Recovery", &recovery, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp))
        SetRecovery(recovery);
    ImGui::PopID();
}
void PlayerUiHealthBar::Destroy()
{
    if (!m_group) return;
    for (auto& widget : m_group->widgets) widget.Destroy();
    *m_group = {};
    m_group = nullptr;
    m_slider = {}; m_label = {}; m_recoveryGauge = {}; m_markers = {};
}
