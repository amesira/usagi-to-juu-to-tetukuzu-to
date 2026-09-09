//===================================================
// File  ：_/UI/Player/player_ui_ammo_count.cpp
// Date  ：2026/09/08
// Author：Miu Kitamura
//===================================================
#include "player_ui_ammo_count.h"

#include "Game/Factory/ui_factory.h"
#include "Engine/Component/image_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/slider_component.h"

#include "External/ImGui/imgui.h"
#include <algorithm>
#include <cmath>
#include <string>

using namespace DirectX;

void PlayerUiAmmoCount::Initialize(IScene* scene, PlayerUi::WidgetGroup& group,
    const PlayerUiSettings::AmmoCountSettings& settings)
{
    if (m_group || !scene) return;

    m_group = &group;
    m_background.Register(group, UiFactory::CreateUiImageHandle(scene,
        L"asset/Texture/Ui/circle_gauge_thin.png"), "PlayerUi.AmmoBackground", 100);
    m_fill.Register(group, UiFactory::CreateUiImageHandle(scene,
        L"asset/Texture/Ui/circle_gauge.png"), "PlayerUi.AmmoFill", 101);

    for (auto* element : {&m_background, &m_fill}) {
        if (auto* image = element->handle.GetImage()) {
            image->SetFillMethod(ImageComponent::FillMethod::RoundFill);
            image->SetFillAmount(0.75f);
        }
    }

    m_background.handle.SetAlpha(0.5f);
    if (auto* image = m_fill.handle.GetImage()) {
        image->SetColor({0.9f, 0.9f, 0.2f, 1});
    }

    m_icon.Register(group, UiFactory::CreateUiImageHandle(scene,
        L"asset/Texture/Ui/dual_pistols_icon.png"), "PlayerUi.WeaponIcon", 102);
    m_currentText.Register(group, UiFactory::CreateUiTextHandle(scene, u8""), "PlayerUi.AmmoCurrent", 104);
    m_capacityText.Register(group, UiFactory::CreateUiTextHandle(scene, u8"/ 20"), "PlayerUi.AmmoCapacity", 104);

    for (auto* element : {&m_currentText, &m_capacityText}) {
        if (auto* text = element->handle.GetText()) {
            text->SetFontSize(22);
            text->SetColor({1, 1, 1, 1});
            text->SetCenter(true);
        }
    }

    const char* names[] = {"PlayerUi.AmmoStart", "PlayerUi.AmmoEnd", "PlayerUi.AmmoFillEdge"};
    for (size_t i = 0; i < m_markers.size(); ++i) {
        m_markers[i].Register(group, UiFactory::CreateUiImageHandle(scene,
            L"asset/Texture/Ui/circle_gause_line.png"), names[i], 103);
    }

    ApplyLayout(settings);
    UpdateDisplay();
}

void PlayerUiAmmoCount::ApplyLayout(const PlayerUiSettings::AmmoCountSettings& settings)
{
    if (!m_group) return;

    m_background.ApplyLayout(*m_group, settings.circleGauge);
    m_fill.ApplyLayout(*m_group, settings.circleGauge);
    m_icon.ApplyLayout(*m_group, settings.weaponIcon);
    m_currentText.ApplyLayout(*m_group, settings.currentText);
    m_capacityText.ApplyLayout(*m_group, settings.capacityText);
    UpdateMarkers();
}

void PlayerUiAmmoCount::SetAmmoCount(int current)
{
    m_ammoCount = std::clamp(current, 0, 20);
    UpdateDisplay();
}

void PlayerUiAmmoCount::UpdateDisplay()
{
    if (!m_group) return;

    if (auto* image = m_fill.handle.GetImage()) {
        image->SetFillAmount(static_cast<float>(m_ammoCount) / 20 * 0.75f);
    }
    if (auto* text = m_currentText.handle.GetText()) {
        text->SetText(std::to_string(m_ammoCount));
    }
    UpdateMarkers();
}

void PlayerUiAmmoCount::UpdateMarkers()
{
    if (!m_group) return;

    for (size_t i = 0; i < m_markers.size(); ++i) {
        const auto& source = i == 2 ? m_fill : m_background;
        auto* rect = source.handle.GetRectTransform();
        auto* image = source.handle.GetImage();
        if (!rect || !image) continue;
        const float fraction = i == 0 ? 0 : image->GetFillAmount();
        const float direction = image->GetFillReverse() ? -1.0f : 1.0f;
        const auto size = rect->GetScaling();
        m_markers[i].ApplyLayout(*m_group, {m_group->offsetPositions[source.slot], {size.x, size.y},
            XMConvertToDegrees(rect->GetRotation().z) + image->GetFillStartAngleDegrees() + direction * fraction * 360});
        if (auto* marker = m_markers[i].handle.GetRectTransform()) {
            marker->SetPresentationTransform(rect->GetPresentationTransform());
        }
    }
}

void PlayerUiAmmoCount::DrawInspector()
{
    ImGui::PushID(this);
    int ammo = m_ammoCount;
    if (ImGui::SliderInt("Ammo / 20", &ammo, 0, 20, "%d", ImGuiSliderFlags_AlwaysClamp)) SetAmmoCount(ammo);
    ImGui::PopID();
}

void PlayerUiAmmoCount::Destroy()
{
    if (!m_group) return;

    for (auto& widget : m_group->widgets) {
        widget.Destroy();
    }

    *m_group = {};
    m_group = nullptr;
    m_background = {}; 
    m_fill = {}; 
    m_icon = {};
    m_currentText = {}; 
    m_capacityText = {}; 
    m_markers = {};
}

void PlayerUiAmmoCount::ApplyColors(const DirectX::XMFLOAT3& color1, const DirectX::XMFLOAT3& color2)
{
    if (!m_group) return;

    for (auto& widget : m_group->widgets) {
        PlayerUiColor::Apply(widget, color2);
    }
    PlayerUiColor::Apply(m_fill.handle, color1);
    PlayerUiColor::Apply(m_currentText.handle, color1);
}
