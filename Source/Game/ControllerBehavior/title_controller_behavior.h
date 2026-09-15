#ifndef TITLE_CONTROLLER_BEHAVIOR_H
#define TITLE_CONTROLLER_BEHAVIOR_H

#include "Engine/Component/behavior_component.h"
#include "title_controller_settings_asset.h"

// タイトルの状態・メニュー進行を担当する。
class TitleControllerBehavior : public BehaviorComponent {
private:
    const TitleControllerSettingsAsset* m_settings = nullptr;
    int m_settingsAssetRevision = -1;
    class TransformComponent* m_titleLogoTransform = nullptr;
    const TitleControllerSettings::Data& Settings() const;
    class TitleUiBehavior* m_titleUi = nullptr;
    class PlayerUiBehavior* m_playerUi = nullptr;
    class OverviewCameraBehavior* m_camera = nullptr;
    int m_selectedMenu = 0;
    enum class State { Menu, ExitConfirm, QuitRequested, StartingGame, EnteringPractice, Practice };
    bool m_exitYesSelected = false;
    State m_state = State::Menu;
public:
    void Setup(const TitleControllerSettingsAsset* settings) { m_settings = settings; }
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    void EnterPractice();
    void StartGame();
    void ReturnToTitle();
    void OpenExitConfirmation();
    void CancelExitConfirmation();
    void ConfirmExitSelection();
};

#endif // TITLE_CONTROLLER_BEHAVIOR_H
