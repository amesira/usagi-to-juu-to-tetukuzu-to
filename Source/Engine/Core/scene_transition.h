#pragma once

// Independent of scenes and rendering resources; survives scene destruction.
class SceneTransition {
public:
    enum class State { Idle, FadeOut, Covered, FadeIn };
    bool Start(float fadeOutDuration = 0.4f, float fadeInDuration = 0.4f);
    void Update(float deltaTime);
    void NotifyFramePresented();
    bool CanSwitchScene() const;
    void BeginFadeIn();
    void Reset();
    bool IsActive() const { return m_state != State::Idle; }
    float GetAlpha() const { return m_alpha; }
    State GetState() const { return m_state; }
private:
    State m_state = State::Idle;
    float m_alpha = 0, m_elapsed = 0;
    float m_fadeOutDuration = 0.4f, m_fadeInDuration = 0.4f;
    bool m_coveredPresented = false;
};
