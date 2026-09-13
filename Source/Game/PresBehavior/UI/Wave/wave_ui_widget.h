#pragma once
#include "wave_ui_settings_asset.h"
#include "Game/PresBehavior/UI/Player/player_ui_widget_element.h"
#include "Game/Factory/ui_factory.h"
#include "Utility/mi_math.h"

// PlayerUiと同じグループ中心＋要素オフセットで配置する、ウェーブ用ウィジェット。
class WaveUiWidget {
    PlayerUi::WidgetGroup m_group;
    PlayerUiWidgetElement m_label;
    PlayerUiWidgetElement m_gauge;
    WaveUiSettings::WidgetSettings m_settings;
    std::string m_lastText;
    bool m_hasGauge = false;
    float m_pulseRemaining = 0;
    float m_targetFill = 0;
    float m_displayFill = 0;
    float m_fillVelocity = 0;
public:
    void Initialize(IScene* scene, const char* name, bool gauge = false) {
        m_label.Register(m_group, UiFactory::CreateUiTextHandle(scene, u8""), name, 110);
        m_hasGauge = gauge;
        if (gauge) m_gauge.Register(m_group, UiFactory::CreateUiSliderHandle(scene, {0.08f, 0.1f, 0.14f, 1}, {1, 1, 1, 1}, 0), "WaveUi.PointGauge", 109);
    }
    void ApplyLayout(const WaveUiSettings::WidgetSettings& settings, DirectX::XMFLOAT2 screen) {
        m_settings = settings;
        m_group.currentCenterPosition = PlayerUiSettings::ResolveGroupPosition(settings.placement, screen);
        m_label.ApplyLayout(m_group, settings.label);
        m_label.handle.SetColor(settings.color);
        if (auto* text = m_label.handle.GetText()) text->SetFontSize(settings.fontSize);
    }
    void ApplyGauge(const WaveUiSettings::PointsSettings& settings) {
        if (!m_hasGauge) return;
        m_gauge.ApplyLayout(m_group, settings.gauge);
        if (auto* gauge = m_gauge.handle.GetSlider()) gauge->SetFillColor({settings.gaugeColor.x, settings.gaugeColor.y, settings.gaugeColor.z, 1});
    }
    void SetText(const std::string& value) {
        if (value == m_lastText) return;
        m_lastText = value;
        if (auto* text = m_label.handle.GetText()) text->SetText(value);
    }
    void SetFill(float value) { m_targetFill = std::isfinite(value) ? std::clamp(value, 0.0f, 1.0f) : 0; }
    void Pulse(float duration) { m_pulseRemaining = duration; }
    void Update(float dt, const WaveUiSettings::Data& settings) {
        if (m_hasGauge && std::isfinite(dt) && dt > 0) {
            m_displayFill = MiMath::SmoothDamp(m_displayFill, m_targetFill,
                m_fillVelocity, settings.points.gaugeSmoothTime, dt);
            m_displayFill = std::clamp(m_displayFill, 0.0f, 1.0f);
            if (std::abs(m_displayFill - m_targetFill) < 0.0001f && std::abs(m_fillVelocity) < 0.0001f) {
                m_displayFill = m_targetFill;
                m_fillVelocity = 0;
            }
            if (auto* gauge = m_gauge.handle.GetSlider()) gauge->SetValue(m_displayFill);
        }
        m_pulseRemaining = (std::max)(0.0f, m_pulseRemaining - dt);
        const float t = settings.pulseDuration > 0 ? std::clamp(m_pulseRemaining / settings.pulseDuration, 0.0f, 1.0f) : 0;
        const float scale = 1 + (settings.pulseScale - 1) * std::sin(t * DirectX::XM_PI);
        m_label.handle.SetSize(m_settings.label.size.x * scale, m_settings.label.size.y * scale);
    }
    void SetActive(bool active) { for (auto& handle : m_group.widgets) handle.SetActive(active); }
    void Destroy() { for (auto& handle : m_group.widgets) handle.Destroy(); *this = {}; }
};
