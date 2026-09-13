#include "scene_transition.h"
#include <algorithm>
#include <cmath>

bool SceneTransition::Start(float outDuration, float inDuration) {
    if (IsActive()) return false;
    m_fadeOutDuration = std::isfinite(outDuration) ? (std::max)(outDuration, 0.0f) : 0.4f;
    m_fadeInDuration = std::isfinite(inDuration) ? (std::max)(inDuration, 0.0f) : 0.4f;
    m_elapsed = 0; m_alpha = 0; m_coveredPresented = false;
    m_state = State::FadeOut;
    return true;
}
void SceneTransition::Update(float deltaTime) {
    if (!std::isfinite(deltaTime) || deltaTime < 0) return;
    if (m_state == State::FadeOut) {
        m_elapsed += deltaTime;
        m_alpha = m_fadeOutDuration > 0 ? (std::min)(m_elapsed / m_fadeOutDuration, 1.0f) : 1.0f;
        if (m_alpha >= 1) { m_state = State::Covered; m_alpha = 1; }
    }
    else if (m_state == State::FadeIn) {
        m_elapsed += deltaTime;
        m_alpha = m_fadeInDuration > 0 ? 1 - (std::min)(m_elapsed / m_fadeInDuration, 1.0f) : 0.0f;
        if (m_alpha <= 0) Reset();
    }
}
void SceneTransition::NotifyFramePresented() {
    if (m_state == State::Covered) m_coveredPresented = true;
}
bool SceneTransition::CanSwitchScene() const { return m_state == State::Covered && m_coveredPresented; }
void SceneTransition::BeginFadeIn() {
    if (!CanSwitchScene()) return;
    m_state = State::FadeIn; m_elapsed = 0; m_alpha = 1;
}
void SceneTransition::Reset() { m_state = State::Idle; m_alpha = 0; m_elapsed = 0; m_coveredPresented = false; }
