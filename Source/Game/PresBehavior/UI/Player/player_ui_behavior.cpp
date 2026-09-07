//===================================================
// File  ：_/PresBehavior/UI/Player/player_ui_behavior.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/image_component.h"
#include "Game/Factory/ui_factory.h"
#include "External/ImGui/imgui.h"

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
    m_presentation.Update(m_context, FPS_GetUnscaledDeltaTime());
}

void PlayerUiBehavior::DrawComponentInspector()
{
    if (!m_widgetsCreated) {
        ImGui::TextUnformatted("Player UI is created when play starts.");
        return;
    }
    const char* labels[] = { "None", "Health", "Ammo", "Crosshair" };
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
}

void PlayerUiBehavior::RegisterWidget(PlayerUi::WidgetGroupID groupID, UiHandle widget,
    const DirectX::XMFLOAT2& offset, const DirectX::XMFLOAT2& size,
    const char* name, float orderInLayer)
{
    auto* object = widget.GetGameObject();
    auto* rect = widget.GetRectTransform();
    if (!object || !rect) return;
    object->SetName(name);
    widget.SetSize(size.x, size.y);
    rect->SetPosition({ 0.0f, 0.0f, orderInLayer });
    auto& group = m_context.widgetGroups[static_cast<size_t>(groupID)];
    group.widgets.push_back(widget);
    group.offsetPositions.push_back(offset);
}

void PlayerUiBehavior::CreateTestWidgets()
{
    if (!m_context.scene || m_widgetsCreated) return;
    using PlayerUi::WidgetGroupID;
    const float width = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float height = static_cast<float>(Direct3D_GetBackBufferHeight());
    const XMFLOAT4 background = { 0.08f, 0.1f, 0.14f, 1.0f };

    RegisterWidget(WidgetGroupID::HealthBar,
        UiFactory::CreateUiSliderHandle(m_context.scene, background, {0.2f, 0.9f, 0.4f, 1.0f}, 0.75f),
        {0, 0}, {280, 20}, "PlayerUi.HealthBar.Test", 100);
    auto healthLabel = UiFactory::CreateUiTextHandle(m_context.scene, u8"HP 75 / 100");
    if (auto* text = healthLabel.GetText()) {
        text->SetFontSize(22);
        text->SetColor({1, 1, 1, 1});
    }
    RegisterWidget(WidgetGroupID::HealthBar, healthLabel, {0, -22}, {1, 1}, "PlayerUi.HealthLabel.Test", 101);

    RegisterWidget(WidgetGroupID::AmmoCount,
        UiFactory::CreateUiSliderHandle(m_context.scene, background, {0.3f, 0.7f, 1.0f, 1.0f}, 0.6f),
        {0, 0}, {220, 16}, "PlayerUi.AmmoBar.Test", 100);
    auto ammoLabel = UiFactory::CreateUiTextHandle(m_context.scene, u8"AMMO 18 / 30");
    if (auto* text = ammoLabel.GetText()) {
        text->SetFontSize(22);
        text->SetColor({1, 1, 1, 1});
    }
    RegisterWidget(WidgetGroupID::AmmoCount, ammoLabel, {0, -20}, {1, 1}, "PlayerUi.AmmoLabel.Test", 101);

    // 既存の白テクスチャを使い、専用アセットなしで十字照準を構成する。
    const XMFLOAT2 offsets[] = { {-10, 0}, {10, 0}, {0, -10}, {0, 10} };
    const XMFLOAT2 sizes[] = { {8, 2}, {8, 2}, {2, 8}, {2, 8} };
    const char* names[] = { "PlayerUi.Crosshair.Left.Test", "PlayerUi.Crosshair.Right.Test",
        "PlayerUi.Crosshair.Top.Test", "PlayerUi.Crosshair.Bottom.Test" };
    for (size_t i = 0; i < 4; ++i) {
        auto image = UiFactory::CreateUiImageHandle(m_context.scene, L"asset/Texture/white.bmp");
        RegisterWidget(WidgetGroupID::Crosshair, image, offsets[i], sizes[i], names[i], 102);
    }

    m_view.SetWidgetGroupPosition(m_context, WidgetGroupID::HealthBar, {180, height - 65});
    m_view.SetWidgetGroupPosition(m_context, WidgetGroupID::AmmoCount, {width - 150, height - 65});
    m_view.SetWidgetGroupPosition(m_context, WidgetGroupID::Crosshair, {width * 0.5f, height * 0.5f});
    m_widgetsCreated = true;
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
