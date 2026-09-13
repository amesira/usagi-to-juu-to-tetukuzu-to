#pragma once
#include "Game/ControllerBehavior/Wave/wave_progress.h"
#include <algorithm>
#include <cmath>

class WaveUiPhaseMotion {
    bool m_moveToB = false;
    float m_age = 0;
public:
    void Begin(WaveProgress::State state) {
        m_moveToB = state == WaveProgress::State::Battle || state == WaveProgress::State::Clearing;
        m_age = 0;
    }
    void Reset() { m_moveToB = false; m_age = 0; }
    void Update(float dt) { if (std::isfinite(dt) && dt > 0) m_age += dt; }
    float GetBlend(float delay, float duration) const {
        if (!m_moveToB || m_age < delay) return 0;
        if (duration <= 0) return 1;
        const float t = std::clamp((m_age - delay) / duration, 0.0f, 1.0f);
        return t * t * (3 - 2 * t);
    }
};
