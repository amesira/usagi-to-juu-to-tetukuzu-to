#pragma once
#include "Engine/Component/behavior_component.h"
#include "Game/PresBehavior/UI/ui_handle.h"
#include "game_pause_ui_settings_asset.h"
#include <array>
#include <cstdint>

class GamePauseUiBehavior : public BehaviorComponent {
    const GamePauseUiSettingsAsset* m_settings = nullptr;
    UiHandle m_dimmer;
    std::array<UiHandle, 5> m_elements;
    bool m_created = false;
    bool m_visible = false;
    bool m_yesSelected = false;
    std::uint64_t m_revision = 0;
    DirectX::XMFLOAT2 m_screenSize = {};
    DirectX::XMFLOAT2 m_selectionStart = {}, m_selectionCurrent = {}, m_selectionTarget = {};
    float m_selectionElapsed = 0, m_selectionDuration = 0;

    const GamePauseUiSettings::Data& Settings() const;
    void ApplySettings();
    void ApplyView();
    DirectX::XMFLOAT2 SelectionTarget() const;
public:
    void Setup(const GamePauseUiSettingsAsset* settings) { m_settings = settings; }
    void Start() override;
    void Update() override;
    void OnDestroy() override;
    void DrawComponentInspector() override;
    void SetVisible(bool visible);
    void SetSelection(bool yesSelected, bool animate = true);
};
