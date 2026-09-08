//===================================================
// File  ：_/PresBehavior/UI/Player/player_ui_behavior.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_behavior.h"
#include "player_ui_settings_asset.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/image_component.h"
#include "Game/Factory/ui_factory.h"
#include "External/ImGui/imgui.h"
#include "Engine/Component/slider_component.h"
#include <algorithm>
#include <cmath>
#include <string>

void PlayerUiBehavior::Start()
{
    if (!GetOwner() || m_widgetsCreated) return;
    m_context.owner = this;
    m_context.scene = GetOwner()->GetScene();

    m_context.view = &m_view;
    m_context.presentation = &m_presentation;
    m_presentation.Initialize();
    CreateTestWidgets();
}

void PlayerUiBehavior::Update()
{
    const auto revision = m_context.settingsAsset ? m_context.settingsAsset->GetRevision() : 0;
    if (m_widgetsCreated && (m_layoutDirty || revision != m_lastSettingsRevision
        || m_lastScreenSize.x != Direct3D_GetBackBufferWidth()
        || m_lastScreenSize.y != Direct3D_GetBackBufferHeight())) {
        ApplyLayoutSettings();
    }
    if (m_widgetsCreated) {
        UpdateHealthMarkers();
        UpdateAmmoMarkers();
    }
    m_presentation.Update(m_context, FPS_GetUnscaledDeltaTime());
}

void PlayerUiBehavior::DrawComponentInspector()
{
    if (!m_widgetsCreated) {
        ImGui::TextUnformatted("Player UI is created when play starts.");
        return;
    }
    ImGui::PushID(this);
    if (ImGui::CollapsingHeader("Gauge Preview", ImGuiTreeNodeFlags_DefaultOpen)) {
        float maximum = m_maxHealth;
        if (ImGui::DragFloat("Max HP", &maximum, 1.0f, 0.0f, 100000.0f,
            "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
            SetHealth(m_health, maximum);
        }
        float health = m_health;
        if (ImGui::SliderFloat("HP", &health, 0.0f, m_maxHealth,
            "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
            // HPテキストとスライダー、Fill位置の白線をまとめて更新する。
            SetHealth(health, m_maxHealth);
        }
        float recovery = m_recovery;
        if (ImGui::SliderFloat("Recovery", &recovery, 0.0f, 1.0f,
            "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            SetRecoveryGauge(recovery);
        }
        int ammo = m_ammoCount;
        if (ImGui::SliderInt("Ammo / 20", &ammo, 0, 20,
            "%d", ImGuiSliderFlags_AlwaysClamp)) {
            SetAmmoCount(ammo);
        }
        if (ImGui::SliderFloat("Remaining Life Fill", &m_remainingLife, 0.0f, 1.0f,
            "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            UpdateDisplayValues();
        }
    }
    ImGui::Separator();
    const char* labels[] = { "None", "Health", "Ammo", "Crosshair", "Remaining Life" };
    for (size_t i = 1; i < static_cast<size_t>(PlayerUi::WidgetGroupID::Max); ++i) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::TextUnformatted(labels[i]);
        const auto id = static_cast<PlayerUi::WidgetGroupID>(i);
        if (ImGui::Button("Shake")) m_presentation.ShakeWidgetGroup(m_context, id, 8.0f, 0.4f);
        ImGui::SameLine();
        if (ImGui::Button("Fade out")) m_presentation.FadeWidgetGroup(m_context, id, 0.0f, 0.3f);
        ImGui::SameLine();
        if (ImGui::Button("Fade in")) m_presentation.FadeWidgetGroup(m_context, id, 1.0f, 0.3f);
        ImGui::PopID();
    }
    ImGui::PopID();
}

void PlayerUiBehavior::RegisterWidget(PlayerUi::WidgetGroupID groupID, UiHandle widget,
    const DirectX::XMFLOAT2& offset, const DirectX::XMFLOAT2& size,
    const char* name, float orderInLayer)
{
    // 設定のスロットとハンドルの対応を固定する（生成失敗時も空ハンドルを登録）。
    auto& group = m_context.widgetGroups[static_cast<size_t>(groupID)];
    group.widgets.push_back(widget);
    group.offsetPositions.push_back(offset);
    auto* object = widget.GetGameObject();
    auto* rect = widget.GetRectTransform();
    if (!object || !rect) return;
    object->SetName(name);
    widget.SetSize(size.x, size.y);
    rect->SetPosition({ 0.0f, 0.0f, orderInLayer });
}

void PlayerUiBehavior::CreateTestWidgets()
{
    if (!m_context.scene || m_widgetsCreated) return;
    using PlayerUi::WidgetGroupID;
    const float width = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float height = static_cast<float>(Direct3D_GetBackBufferHeight());
    static const PlayerUiSettings::Data defaults;
    const auto& settings = m_context.settingsAsset ? m_context.settingsAsset->GetData() : defaults;
    const XMFLOAT4 background = { 0.08f, 0.1f, 0.14f, 1.0f };

    const auto addText = [&](WidgetGroupID group, const PlayerUiSettings::WidgetTransform& t,
        const char* name, const char8_t* value) {
        auto handle = UiFactory::CreateUiTextHandle(m_context.scene, value);
        if (auto* text = handle.GetText()) {
            text->SetFontSize(22);
            text->SetColor({1, 1, 1, 1});
            text->SetCenter(true);
        }
        RegisterWidget(group, handle, t.position, t.size, name, 104);
    };
    const auto addImage = [&](WidgetGroupID group, const PlayerUiSettings::WidgetTransform& t,
        const wchar_t* path, const char* name, float layer, ImageComponent::FillMethod fill) {
        auto handle = UiFactory::CreateUiImageHandle(m_context.scene, path);
        if (auto* image = handle.GetImage()) {
            image->SetFillMethod(fill);
            image->SetFillAmount(1);
            image->SetColor({1, 1, 1, 1});
        }
        RegisterWidget(group, handle, t.position, t.size, name, layer);

        return handle;
    };
    using Fill = ImageComponent::FillMethod;
    // Health slots: slider, text, recovery, left marker, right marker, fill marker.
    RegisterWidget(WidgetGroupID::HealthBar,
        UiFactory::CreateUiSliderHandle(m_context.scene, background, {0.2f, 0.9f, 0.4f, 1}, 1),
        settings.healthBar.slider.position, settings.healthBar.slider.size, "PlayerUi.HealthBar", 100);
    addText(WidgetGroupID::HealthBar, settings.healthBar.label, "PlayerUi.HealthText", u8"1000/1000");
    if (auto* text = m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::HealthBar)].widgets[1].GetText()) text->SetCenter(false);
    addImage(WidgetGroupID::HealthBar, settings.healthBar.recoveryGauge,
        L"asset/Texture/Ui/recovery_gauge.png", "PlayerUi.RecoveryGauge", 100, Fill::Horizontal);
    for (const char* name : {"PlayerUi.HealthLeft", "PlayerUi.HealthRight", "PlayerUi.HealthFill"}) {
        addImage(WidgetGroupID::HealthBar, {}, L"asset/Texture/white.bmp", name, 103, Fill::None);
    }
    // Ammo slots: background ring, fill ring, icon, current text, capacity text.
    UiHandle circleGauge = addImage(WidgetGroupID::AmmoCount, settings.ammoCount.circleGauge,
        L"asset/Texture/Ui/circle_gauge_thin.png", "PlayerUi.AmmoBackground", 100, Fill::RoundFill);
    circleGauge.GetImage()->SetFillAmount(0.75f);
    circleGauge.SetAlpha(0.5f);
    circleGauge = addImage(WidgetGroupID::AmmoCount, settings.ammoCount.circleGauge,
        L"asset/Texture/Ui/circle_gauge.png", "PlayerUi.AmmoFill", 101, Fill::RoundFill);
    circleGauge.GetImage()->SetFillAmount(0.75f);
    circleGauge.GetImage()->SetColor({ 0.9f, 0.9f, 0.2f, 1 });

    addImage(WidgetGroupID::AmmoCount, settings.ammoCount.weaponIcon,
        L"asset/Texture/Ui/dual_pistols_icon.png", "PlayerUi.WeaponIcon", 102, Fill::None);
    addText(WidgetGroupID::AmmoCount, settings.ammoCount.currentText, "PlayerUi.AmmoCurrent", u8"20");
    addText(WidgetGroupID::AmmoCount, settings.ammoCount.capacityText, "PlayerUi.AmmoCapacity", u8"/20");
    // Ammo slots 5-7: start, end of the background arc, current fill edge.
    for (const char* name : {"PlayerUi.AmmoStart", "PlayerUi.AmmoEnd", "PlayerUi.AmmoFillEdge"}) {
        addImage(WidgetGroupID::AmmoCount, settings.ammoCount.circleGauge,
            L"asset/Texture/Ui/circle_gause_line.png", name, 103, Fill::None);
    }
    addImage(WidgetGroupID::RemainingLife, settings.remainingLife.gauge,
        L"asset/Texture/Ui/player_remaining_life_gauge.png", "PlayerUi.RemainingLife", 100, Fill::Horizontal);

    // 既存の白テクスチャを使い、専用アセットなしで十字照準を構成する。
    const XMFLOAT2 offsets[] = { {-10, 0}, {10, 0}, {0, -10}, {0, 10} };
    const XMFLOAT2 sizes[] = { {8, 2}, {8, 2}, {2, 8}, {2, 8} };
    const char* names[] = { "PlayerUi.Crosshair.Left.Test", "PlayerUi.Crosshair.Right.Test",
        "PlayerUi.Crosshair.Top.Test", "PlayerUi.Crosshair.Bottom.Test" };
    for (size_t i = 0; i < 4; ++i) {
        auto image = UiFactory::CreateUiImageHandle(m_context.scene, L"asset/Texture/white.bmp");
        RegisterWidget(WidgetGroupID::Crosshair, image, offsets[i], sizes[i], names[i], 102);
    }

    m_view.SetWidgetGroupPosition(m_context, WidgetGroupID::Crosshair, {width * 0.5f, height * 0.5f});
    m_widgetsCreated = true;
    ApplyLayoutSettings();
    UpdateDisplayValues();
}

void PlayerUiBehavior::DestroyWidgets()
{
    m_presentation.CancelAll(m_context);
    for (auto& group : m_context.widgetGroups) {
        for (auto& widget : group.widgets) widget.Destroy();
        group = {};
    }
    m_widgetsCreated = false;
}

void PlayerUiBehavior::Setup(const PlayerUiSettingsAsset* settingsAsset)
{
    m_context.settingsAsset = settingsAsset;
    m_layoutDirty = true;
    if (m_widgetsCreated) ApplyLayoutSettings();
}

void PlayerUiBehavior::ApplyLayoutSettings()
{
    static const PlayerUiSettings::Data defaults;
    const auto& settings = m_context.settingsAsset ? m_context.settingsAsset->GetData() : defaults;
    const XMFLOAT2 screenSize = {static_cast<float>(Direct3D_GetBackBufferWidth()),
        static_cast<float>(Direct3D_GetBackBufferHeight())};
    const auto applyGroup = [&](PlayerUi::WidgetGroupID id, const PlayerUiSettings::GroupPlacement& placement,
        std::initializer_list<const PlayerUiSettings::WidgetTransform*> transforms) {
        auto& group = m_context.widgetGroups[static_cast<size_t>(id)];
        size_t index = 0;
        for (const auto* transform : transforms) {
            if (index >= group.widgets.size() || index >= group.offsetPositions.size()) break;
            group.offsetPositions[index] = transform->position;
            auto& widget = group.widgets[index++];
            widget.SetSize(transform->size.x, transform->size.y);
            if (auto* rect = widget.GetRectTransform()) {
                rect->SetRotation({0, 0, XMConvertToRadians(transform->rotationDegrees)});
            }
        }
        // Viewが現在のShakeオフセットを合成するため、再配置中も演出を維持できる。
        m_view.SetWidgetGroupPosition(m_context, id, PlayerUiSettings::ResolveGroupPosition(placement, screenSize));
    };
    applyGroup(PlayerUi::WidgetGroupID::HealthBar, settings.healthBar.placement,
        {&settings.healthBar.slider, &settings.healthBar.label, &settings.healthBar.recoveryGauge});
    applyGroup(PlayerUi::WidgetGroupID::AmmoCount, settings.ammoCount.placement,
        {&settings.ammoCount.circleGauge, &settings.ammoCount.circleGauge,
         &settings.ammoCount.weaponIcon, &settings.ammoCount.currentText, &settings.ammoCount.capacityText});
    applyGroup(PlayerUi::WidgetGroupID::RemainingLife, settings.remainingLife.placement,
        {&settings.remainingLife.gauge});
    UpdateHealthMarkers();
    UpdateAmmoMarkers();
    m_lastScreenSize = screenSize;
    m_lastSettingsRevision = m_context.settingsAsset ? m_context.settingsAsset->GetRevision() : 0;
    m_layoutDirty = false;
}


// All display setters also work before widget creation.
void PlayerUiBehavior::SetHealth(float current, float maximum)
{
    m_maxHealth = std::isfinite(maximum) ? (std::max)(maximum, 0.0f) : 0;
    m_health = std::isfinite(current) ? std::clamp(current, 0.0f, m_maxHealth) : 0;
    UpdateDisplayValues();
}
void PlayerUiBehavior::SetAmmoCount(int current)
{
    m_ammoCount = std::clamp(current, 0, 20);
    UpdateDisplayValues();
}
void PlayerUiBehavior::SetRecoveryGauge(float amount)
{
    m_recovery = std::isfinite(amount) ? std::clamp(amount, 0.0f, 1.0f) : 0;
    UpdateDisplayValues();
}
void PlayerUiBehavior::SetRemainingLife(int current, int maximum)
{
    m_remainingLife = maximum > 0 ? std::clamp(static_cast<float>(current) / maximum, 0.0f, 1.0f) : 0;
    UpdateDisplayValues();
}
void PlayerUiBehavior::UpdateDisplayValues()
{
    if (!m_widgetsCreated) return;
    auto& hp = m_context.widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::HealthBar)].widgets;
    auto& ammo = m_context.widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::AmmoCount)].widgets;
    auto& life = m_context.widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::RemainingLife)].widgets;
    if (auto* slider = hp[0].GetSlider()) slider->SetValue(m_maxHealth > 0 ? m_health / m_maxHealth : 0);
    if (auto* text = hp[1].GetText()) text->SetText(std::to_string(static_cast<int>(std::ceil(m_health))) + " / " + std::to_string(static_cast<int>(std::ceil(m_maxHealth))));
    if (auto* image = hp[2].GetImage()) image->SetFillAmount(m_recovery);
    if (auto* image = ammo[1].GetImage()) image->SetFillAmount(static_cast<float>(m_ammoCount) / 20 * 0.75f);
    if (auto* text = ammo[3].GetText()) text->SetText(std::to_string(m_ammoCount));
    if (auto* image = life[0].GetImage()) image->SetFillAmount(m_remainingLife);
    UpdateHealthMarkers();
    UpdateAmmoMarkers();
}
void PlayerUiBehavior::UpdateHealthMarkers()
{
    auto& group = m_context.widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::HealthBar)];
    if (group.widgets.size() < 6) return;
    auto* rect = group.widgets[0].GetRectTransform();
    auto* slider = group.widgets[0].GetSlider();
    if (!rect || !slider) return;
    const auto size = rect->GetScaling();
    const float angle = rect->GetRotation().z;
    const PlayerUiSettings::WidgetTransform bar = {
        group.offsetPositions[0], {size.x, size.y}, XMConvertToDegrees(angle)};
    const float fractions[] = {0, 1, slider->GetValue()};
    for (size_t i = 0; i < 3; ++i) {
        const auto transform = PlayerUiSettings::MakeHealthMarker(bar, fractions[i]);
        const auto offset = transform.position;
        group.offsetPositions[i + 3] = offset;
        auto& marker = group.widgets[i + 3];
        marker.SetSize(transform.size.x, transform.size.y);
        marker.SetPosition(group.currentCenterPosition.x + offset.x, group.currentCenterPosition.y + offset.y);
        if (auto* markerRect = marker.GetRectTransform()) {
            markerRect->SetRotation({0, 0, angle});
            markerRect->SetPresentationTransform(rect->GetPresentationTransform());
        }
    }
}


void PlayerUiBehavior::UpdateAmmoMarkers()
{
    auto& group = m_context.widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::AmmoCount)];
    if (group.widgets.size() < 8 || group.offsetPositions.size() < 8) return;
    for (size_t i = 0; i < 3; ++i) {
        // Fixed boundaries follow the full background arc; the moving line follows the fill.
        const size_t sourceIndex = i == 2 ? 1 : 0;
        auto* rect = group.widgets[sourceIndex].GetRectTransform();
        auto* image = group.widgets[sourceIndex].GetImage();
        if (!rect || !image) continue;
        const float fraction = i == 0 ? 0.0f : image->GetFillAmount();
        const float direction = image->GetFillReverse() ? -1.0f : 1.0f;
        const float angle = rect->GetRotation().z + XMConvertToRadians(
            image->GetFillStartAngleDegrees() + direction * fraction * 360.0f);
        auto& marker = group.widgets[i + 5];
        group.offsetPositions[i + 5] = group.offsetPositions[sourceIndex];
        const auto position = rect->GetPosition();
        const auto size = rect->GetScaling();
        marker.SetPosition(position.x, position.y);
        marker.SetSize(size.x, size.y);
        if (auto* markerRect = marker.GetRectTransform()) {
            markerRect->SetRotation({0, 0, angle});
            markerRect->SetPresentationTransform(rect->GetPresentationTransform());
        }
    }
}
