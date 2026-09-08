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
#include "Engine/Component/image_component.h"
#include "Game/Factory/ui_factory.h"
#include "External/ImGui/imgui.h"
#include <algorithm>
#include <cmath>

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
    m_healthBar.Initialize(m_context.scene,
        m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::HealthBar)], settings.healthBar);
    m_ammoCount.Initialize(m_context.scene,
        m_context.widgetGroups[static_cast<size_t>(WidgetGroupID::AmmoCount)], settings.ammoCount);
    const auto addImage = [&](WidgetGroupID group, const PlayerUiSettings::WidgetTransform& t,
        const wchar_t* path, const char* name, float layer, ImageComponent::FillMethod fill) {
        auto handle = UiFactory::CreateUiImageHandle(m_context.scene, path);
        if (auto* image = handle.GetImage()) image->SetFillMethod(fill);
        RegisterWidget(group, handle, t.position, t.size, name, layer);
    };
    using Fill = ImageComponent::FillMethod;
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
    m_healthBar.Destroy();
    m_ammoCount.Destroy();
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
    m_healthBar.ApplyLayout(settings.healthBar);
    m_ammoCount.ApplyLayout(settings.ammoCount);
    m_view.SetWidgetGroupPosition(m_context, PlayerUi::WidgetGroupID::HealthBar,
        PlayerUiSettings::ResolveGroupPosition(settings.healthBar.placement, screenSize));
    m_view.SetWidgetGroupPosition(m_context, PlayerUi::WidgetGroupID::AmmoCount,
        PlayerUiSettings::ResolveGroupPosition(settings.ammoCount.placement, screenSize));
    applyGroup(PlayerUi::WidgetGroupID::RemainingLife, settings.remainingLife.placement,
        {&settings.remainingLife.gauge});
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
    m_remainingLife = maximum > 0 ? std::clamp(static_cast<float>(current) / maximum, 0.0f, 1.0f) : 0;
    UpdateDisplayValues();
}
void PlayerUiBehavior::UpdateDisplayValues()
{
    if (!m_widgetsCreated) return;
    auto& life = m_context.widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::RemainingLife)].widgets;
    if (auto* image = life[0].GetImage()) image->SetFillAmount(m_remainingLife);
}
