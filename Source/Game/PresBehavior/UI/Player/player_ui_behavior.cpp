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
#include "External/ImGui/imgui.h"

void PlayerUiBehavior::Start()
{
    if (!GetOwner() || m_widgetsCreated) return;
    m_context.owner = this;
    m_context.scene = GetOwner()->GetScene();

    m_context.view = &m_view;
    m_context.presentation = &m_presentation;
    m_presentation.Initialize();
    CreateWidgets();
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
        m_healthBar.UpdateMarkers();
        m_ammoCount.UpdateMarkers();
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
        m_healthBar.DrawInspector();
        m_ammoCount.DrawInspector();
        m_remainingLife.DrawInspector();
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

void PlayerUiBehavior::CreateWidgets()
{
    if (!m_context.scene || m_widgetsCreated) return;
    using PlayerUi::WidgetGroupID;
    static const PlayerUiSettings::Data defaults;
    const auto& settings = m_context.settingsAsset ? m_context.settingsAsset->GetData() : defaults;
    m_healthBar.Initialize(m_context.scene,
        m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::HealthBar)], settings.healthBar);
    m_ammoCount.Initialize(m_context.scene,
        m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::AmmoCount)], settings.ammoCount);
    m_remainingLife.Initialize(m_context.scene,
        m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::RemainingLife)], settings.remainingLife);
    m_crosshair.Initialize(m_context.scene,
        m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::Crosshair)]);
    m_widgetsCreated = true;
    ApplyLayoutSettings();
}

void PlayerUiBehavior::DestroyWidgets()
{
    m_presentation.CancelAll(m_context);
    m_healthBar.Destroy();
    m_ammoCount.Destroy();
    m_remainingLife.Destroy();
    m_crosshair.Destroy();
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
    m_healthBar.ApplyColors(settings.color1, settings.color2);
    m_ammoCount.ApplyColors(settings.color1, settings.color2);
    m_remainingLife.ApplyColors(settings.color2);
    m_crosshair.ApplyColors(settings.color2);
    m_healthBar.ApplyLayout(settings.healthBar);
    m_ammoCount.ApplyLayout(settings.ammoCount);
    m_remainingLife.ApplyLayout(settings.remainingLife);
    m_crosshair.ApplyLayout();
    m_view.SetWidgetGroupPosition(m_context, PlayerUi::WidgetGroupID::HealthBar,
        PlayerUiSettings::ResolveGroupPosition(settings.healthBar.placement, screenSize));
    m_view.SetWidgetGroupPosition(m_context, PlayerUi::WidgetGroupID::AmmoCount,
        PlayerUiSettings::ResolveGroupPosition(settings.ammoCount.placement, screenSize));
    m_view.SetWidgetGroupPosition(m_context, PlayerUi::WidgetGroupID::RemainingLife,
        PlayerUiSettings::ResolveGroupPosition(settings.remainingLife.placement, screenSize));
    m_view.SetWidgetGroupPosition(m_context, PlayerUi::WidgetGroupID::Crosshair,
        {screenSize.x * 0.5f, screenSize.y * 0.5f});
    m_healthBar.UpdateMarkers();
    m_ammoCount.UpdateMarkers();
    m_lastScreenSize = screenSize;
    m_lastSettingsRevision = m_context.settingsAsset ? m_context.settingsAsset->GetRevision() : 0;
    m_layoutDirty = false;
}


// All display setters also work before widget creation.
void PlayerUiBehavior::SetHealth(float current, float maximum)
{
    m_healthBar.SetHealth(current, maximum);
}
void PlayerUiBehavior::SetAmmoCount(int current)
{
    m_ammoCount.SetAmmoCount(current);
}
void PlayerUiBehavior::SetRecoveryGauge(float amount)
{
    m_healthBar.SetRecovery(amount);
}
void PlayerUiBehavior::SetRemainingLife(int current, int maximum)
{
    m_remainingLife.SetRemainingLife(current, maximum);
}
