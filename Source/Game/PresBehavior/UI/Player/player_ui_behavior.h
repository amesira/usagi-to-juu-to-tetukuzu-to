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
#include "player_ui_health_bar.h"
#include "player_ui_ammo_count.h"
#include "player_ui_crosshair.h"
#include "player_ui_remaining_life.h"

class PlayerUiSettingsAsset;

class PlayerUiBehavior : public BehaviorComponent {
private:
    PlayerUiContext m_context;

    PlayerUiView m_view;
    PlayerUiPresentation m_presentation;

    bool m_widgetsCreated = false;
    PlayerUiHealthBar m_healthBar;
    PlayerUiAmmoCount m_ammoCount;
    PlayerUiCrosshair m_crosshair;
    PlayerUiRemainingLife m_remainingLife;

    std::uint64_t m_lastSettingsRevision = 0;
    DirectX::XMFLOAT2 m_lastScreenSize = {};
    bool m_layoutDirty = true;

    DirectX::XMFLOAT2 m_perspectiveVanishingPointOffset = {};
    float m_perspectiveCameraDistanceMultiplier = 1.0f;

public:
    PlayerUiBehavior() = default;
    ~PlayerUiBehavior() = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // シーン全体の破棄はシーンが担当。個別にUIを終了するときに呼ぶ。
    void DestroyWidgets();

    // === 外部からのUI更新関数 ===
    void Setup(const PlayerUiSettingsAsset* settingsAsset);
    void SetHealth(float current, float maximum);
    void SetAmmoCount(int current);
    void SetRecoveryGauge(float amount);
    void SetRemainingLife(int current, int maximum);

    void SetPerspectiveVanishingPointOffset(const DirectX::XMFLOAT2& offset) {
        m_perspectiveVanishingPointOffset = offset;
        m_layoutDirty = true;
    }
    void SetPerspectiveCameraDistanceMultiplier(float multiplier) {
        m_perspectiveCameraDistanceMultiplier = multiplier;
        m_layoutDirty = true;
    }

private:
    void UpdateRuntimeState();
    void CreateWidgets();
    void ApplyLayoutSettings();

};
