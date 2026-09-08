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

    RegisterWidget(WidgetGroupID::HealthBar,
        UiFactory::CreateUiSliderHandle(m_context.scene, background, {0.2f, 0.9f, 0.4f, 1.0f}, 0.75f),
        settings.healthBar.slider.position, settings.healthBar.slider.size, "PlayerUi.HealthBar.Test", 100);
    auto healthLabel = UiFactory::CreateUiTextHandle(m_context.scene, u8"HP 75 / 100");
    if (auto* text = healthLabel.GetText()) {
        text->SetFontSize(22);
        text->SetColor({1, 1, 1, 1});
        text->SetCenter(false);
    }
    RegisterWidget(WidgetGroupID::HealthBar, healthLabel, settings.healthBar.label.position, settings.healthBar.label.size, "PlayerUi.HealthLabel.Test", 101);

    RegisterWidget(WidgetGroupID::AmmoCount,
        UiFactory::CreateUiSliderHandle(m_context.scene, background, {0.3f, 0.7f, 1.0f, 1.0f}, 0.6f),
        settings.ammoCount.slider.position, settings.ammoCount.slider.size, "PlayerUi.AmmoBar.Test", 103);
    auto ammoLabel = UiFactory::CreateUiTextHandle(m_context.scene, u8"AMMO 18 / 30");
    if (auto* text = ammoLabel.GetText()) {
        text->SetFontSize(22);
        text->SetColor({1, 1, 1, 1});
        text->SetCenter(false);
    }
    RegisterWidget(WidgetGroupID::AmmoCount, ammoLabel, settings.ammoCount.label.position, settings.ammoCount.label.size, "PlayerUi.AmmoLabel.Test", 103);
    auto ammoBar = UiFactory::CreateUiImageHandle(m_context.scene, L"asset/Texture/ammo_slider.png");
    RegisterWidget(WidgetGroupID::AmmoCount, ammoBar,
        settings.ammoCount.fillImage.position, settings.ammoCount.fillImage.size, "PlayerUi.AmmoBar2.Test", 102);
    if (auto* image = ammoBar.GetImage()) {
        image->SetFillMethod(ImageComponent::FillMethod::Horizontal);
        image->SetFillReverse(false);
        image->SetFillAmount(0.6f);
        image->SetColor({ 0.3f, 0.7f, 1.0f, 1.0f });
    }
    auto ammoBgBar = UiFactory::CreateUiImageHandle(m_context.scene, L"asset/Texture/ammo_slider.png");
    RegisterWidget(WidgetGroupID::AmmoCount, ammoBgBar,
        settings.ammoCount.backgroundImage.position, settings.ammoCount.backgroundImage.size, "PlayerUi.AmmoBarBg.Test", 101);
    if (auto* image = ammoBgBar.GetImage()) {
        image->SetColor({ 0.08f, 0.1f, 0.14f, 1.0f });
    }

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
        {&settings.healthBar.slider, &settings.healthBar.label});
    applyGroup(PlayerUi::WidgetGroupID::AmmoCount, settings.ammoCount.placement,
        {&settings.ammoCount.slider, &settings.ammoCount.label,
         &settings.ammoCount.fillImage, &settings.ammoCount.backgroundImage});
    m_lastScreenSize = screenSize;
    m_lastSettingsRevision = m_context.settingsAsset ? m_context.settingsAsset->GetRevision() : 0;
    m_layoutDirty = false;
}
