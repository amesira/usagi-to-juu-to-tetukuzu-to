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
#include "Engine/engine_service_locator.h"

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
    m_group->applyPerspective = true;

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
    m_settings = settings;
    ApplyWeaponTexture();
    ApplyIconOffset();
    m_currentText.ApplyLayout(*m_group, settings.currentText);
    m_capacityText.ApplyLayout(*m_group, settings.capacityText);
    UpdateMarkers();
}

void PlayerUiAmmoCount::SetAmmoCount(int current, int maximum)
{
    m_ammoCapacity = (std::max)(maximum, 0);
    m_ammoCount = std::clamp(current, 0, m_ammoCapacity);
    UpdateDisplay();
}

void PlayerUiAmmoCount::SetWeaponDisplay(PlayerUi::WeaponDisplayType type, bool animate)
{
    const bool changed = type != m_weaponDisplay;
    m_weaponDisplay = type;
    ApplyWeaponTexture();
    if (changed && animate && m_group) {
        m_iconShake.m_intensity = m_settings.iconShakeIntensity;
        m_iconShake.m_duration = m_settings.iconShakeDuration;
        m_iconShake.Start();
    }
    else if (!animate) {
        m_iconShake.Cancel();
    }
    ApplyIconOffset();
}

void PlayerUiAmmoCount::ApplyWeaponTexture()
{
    if (!m_group || !TEXTURE_REPOSITORY) return;
    const std::string* path = &m_settings.dualPistolsIconPath;
    if (m_weaponDisplay == PlayerUi::WeaponDisplayType::Shotgun)
        path = &m_settings.shotgunIconPath;
    else if (m_weaponDisplay == PlayerUi::WeaponDisplayType::SlashBurst)
        path = &m_settings.slashBurstIconPath;
    if (auto* image = m_icon.handle.GetImage()) {
        if (auto* texture = TEXTURE_REPOSITORY->GetTextureResource(*path))
            image->SetTextureResource(texture);
    }
}

void PlayerUiAmmoCount::ApplyIconOffset()
{
    if (!m_group || m_icon.slot >= m_group->offsetPositions.size()) return;
    const auto base = m_settings.weaponIcon.position;
    const auto shake = m_iconShake.m_currentOffset;
    // Viewもこの位置を使うため、グループ演出や透視変換で上書きされない。
    m_group->offsetPositions[m_icon.slot] = {base.x + shake.x, base.y + shake.y};
    m_icon.handle.SetPosition(m_group->currentCenterPosition.x + base.x + shake.x,
        m_group->currentCenterPosition.y + base.y + shake.y);
}

void PlayerUiAmmoCount::Update(float deltaTime)
{
    if (!m_group) return;
    m_iconShake.Update(deltaTime);
    ApplyIconOffset();
    UpdateMarkers();
}

void PlayerUiAmmoCount::UpdateDisplay()
{
    if (!m_group) return;

    if (auto* image = m_fill.handle.GetImage()) {
        const float rate = m_ammoCapacity > 0
            ? static_cast<float>(m_ammoCount) / m_ammoCapacity
            : 0.0f;
        image->SetFillAmount(rate * 0.75f);
    }
    if (auto* text = m_currentText.handle.GetText()) {
        text->SetText(std::to_string(m_ammoCount));
    }
    if (auto* text = m_capacityText.handle.GetText()) {
        text->SetText("/ " + std::to_string(m_ammoCapacity));
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
    if (ImGui::SliderInt("Ammo", &ammo, 0, m_ammoCapacity, "%d", ImGuiSliderFlags_AlwaysClamp))
        SetAmmoCount(ammo, m_ammoCapacity);
    ImGui::PopID();
}

void PlayerUiAmmoCount::Destroy()
{
    m_iconShake.Cancel();
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
        widget.SetColor(color2);
    }

    m_fill.handle.SetColor(color1);
    m_currentText.handle.SetColor(color1);
}
