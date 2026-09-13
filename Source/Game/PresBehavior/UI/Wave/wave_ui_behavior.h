#pragma once
#include "Engine/Component/behavior_component.h"
#include "wave_ui_widget.h"
#include "Game/ControllerBehavior/Wave/wave_progress.h"
#include <array>
#include <cstdint>

class WaveControllerBehavior;
class WaveUiBehavior : public BehaviorComponent {
    struct Popup {
        UiHandle handle;
        DirectX::XMFLOAT3 origin = {};
        float age = 0;
        float duration = 0.8f;
        float rise = 1.5f;
        bool active = false;
    };
    const WaveUiSettingsAsset* m_settingsAsset = nullptr;
    WaveUiSettings::Data m_settings;
    WaveUiWidget m_number, m_points, m_phase;
    std::array<Popup, 24> m_popups;
    size_t m_nextPopup = 0;
    bool m_created = false;
    bool m_dirty = true;
    std::uint64_t m_revision = 0, m_eventCursor = 0;
    DirectX::XMFLOAT2 m_screen = {};
    const WaveControllerBehavior* m_controller = nullptr;
    WaveProgress::State m_lastState = WaveProgress::State::WaitingForWorld;
    int m_lastWave = -1, m_lastPoints = -1;
public:
    void Setup(const WaveUiSettingsAsset* settings) { m_settingsAsset = settings; m_dirty = true; }
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    void DestroyWidgets();
private:
    void ApplyLayout();
    void SpawnPopup(int points, DirectX::XMFLOAT3 position);
    void UpdatePopups(float dt);
};
