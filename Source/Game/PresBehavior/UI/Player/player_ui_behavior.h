//---------------------------------------------------
// File  ：_/PresBehavior/UI/Player/player_ui_behavior.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・PlayerUiを制御するためのBehavior
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"

#include "Game/PresBehavior/UI/ui_handle.h"
#include "player_ui_context.h"
#include "player_ui_presentation.h"
#include "player_ui_view.h"
#include <cstdint>

class PlayerUiSettingsAsset;

class PlayerUiBehavior : public BehaviorComponent {
private:
    PlayerUiContext m_context;

    PlayerUiView m_view;
    PlayerUiPresentation m_presentation;
    bool m_widgetsCreated = false;
    std::uint64_t m_lastSettingsRevision = 0;
    DirectX::XMFLOAT2 m_lastScreenSize = {};
    bool m_layoutDirty = true;

public:
    PlayerUiBehavior() = default;
    ~PlayerUiBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    // シーン全体の破棄はシーンが担当。個別にUIを終了するときに呼ぶ。
    void DestroyWidgets();
    void Setup(const PlayerUiSettingsAsset* settingsAsset);

private:
    void CreateTestWidgets();
    void ApplyLayoutSettings();
    void RegisterWidget(PlayerUi::WidgetGroupID groupID, UiHandle widget,
        const DirectX::XMFLOAT2& offset, const DirectX::XMFLOAT2& size,
        const char* name, float orderInLayer);

};
