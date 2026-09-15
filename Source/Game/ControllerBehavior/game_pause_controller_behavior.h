#pragma once
#include "Engine/Component/behavior_component.h"

class GamePauseUiBehavior;

class GamePauseControllerBehavior : public BehaviorComponent {
    GamePauseUiBehavior* m_ui = nullptr;
    bool m_open = false;
    bool m_yesSelected = false;
    bool m_changingScene = false;
    float m_previousTimeScale = 1.0f;

    bool CanOpen() const;
    void Open();
    void Close();
public:
    ~GamePauseControllerBehavior() override;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    bool IsOpen() const { return m_open; }
};
