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
    float m_health = 100, m_maxHealth = 100;
    float m_recovery = 1, m_remainingLife = 1;
    int m_ammoCount = 20;

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
    void SetHealth(float current, float maximum);
    void SetAmmoCount(int current);
    void SetRecoveryGauge(float amount);
    void SetRemainingLife(int current, int maximum);


private:
    void CreateTestWidgets();
    void ApplyLayoutSettings();
    void UpdateDisplayValues();
    void UpdateHealthMarkers();
    void RegisterWidget(PlayerUi::WidgetGroupID groupID, UiHandle widget,
        const DirectX::XMFLOAT2& offset, const DirectX::XMFLOAT2& size,
        const char* name, float orderInLayer);

};
