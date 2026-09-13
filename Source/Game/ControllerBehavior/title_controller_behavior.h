#ifndef TITLE_CONTROLLER_BEHAVIOR_H
#define TITLE_CONTROLLER_BEHAVIOR_H

#include "Engine/Component/behavior_component.h"

// タイトルの状態・メニュー進行を担当する。
class TitleControllerBehavior : public BehaviorComponent {
private:
    class TitleUiBehavior* m_titleUi = nullptr;
    class PlayerUiBehavior* m_playerUi = nullptr;
    class TitleCameraBehavior* m_camera = nullptr;
    int m_selectedMenu = 0;
    enum class State { Menu, ExitConfirm, QuitRequested, EnteringPractice, Practice };
    bool m_exitYesSelected = false;
    State m_state = State::Menu;
public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    void EnterPractice();
    void ReturnToTitle();
    void OpenExitConfirmation();
    void CancelExitConfirmation();
    void ConfirmExitSelection();
};

#endif // TITLE_CONTROLLER_BEHAVIOR_H
