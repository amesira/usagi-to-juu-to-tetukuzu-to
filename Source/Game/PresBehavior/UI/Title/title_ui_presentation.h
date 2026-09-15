#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

// A selection can be redirected while an earlier movement is still running.
class TitleUiSelectionMotion {
    DirectX::XMFLOAT2 m_start = {}, m_current = {}, m_target = {};
    float m_elapsed = 0, m_duration = 0;
public:
    void MoveTo(DirectX::XMFLOAT2 target, float duration) {
        m_start = m_current; m_target = target; m_elapsed = 0;
        m_duration = std::isfinite(duration) ? (std::max)(duration, 0.0f) : 0.0f;
        if (m_duration == 0) m_current = target;
    }
    void Update(float deltaTime) {
        if (m_duration <= 0 || !std::isfinite(deltaTime) || deltaTime < 0) return;
        m_elapsed = (std::min)(m_elapsed + deltaTime, m_duration);
        float t = m_elapsed / m_duration;
        t = t * t * (3 - 2 * t);
        m_current = {m_start.x + (m_target.x - m_start.x) * t, m_start.y + (m_target.y - m_start.y) * t};
    }
    DirectX::XMFLOAT2 GetPosition() const { return m_current; }
};
struct TitleUiPresentation {
    TitleUiSelectionMotion menuSelection;
    TitleUiSelectionMotion popupSelection;
    void Update(float deltaTime) { menuSelection.Update(deltaTime); popupSelection.Update(deltaTime); }
};
