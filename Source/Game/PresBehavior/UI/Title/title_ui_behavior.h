#pragma once
#include "Engine/Component/behavior_component.h"
#include "title_ui_widgets.h"
#include "title_ui_view.h"
#include "title_ui_presentation.h"
#include <cstdint>

class TitleUiBehavior : public BehaviorComponent {
    const TitleUiSettingsAsset* m_settings = nullptr;
    TitleUiMenu m_menu;
    TitleUiExitPopup m_popup;
    TitleUiVersion m_version;
    TitleUiHighScore m_highScore;
    TitleUiView m_view;
    TitleUiPresentation m_presentation;
    TitleUi::MenuItem m_selected = TitleUi::MenuItem::Practice;
    bool m_yesSelected = false, m_created = false, m_dirty = true;
    bool m_menuVisible = true, m_popupVisible = false;
    bool m_visible = true;
    int m_score = 0;
    std::string m_versionOverride;
    bool m_hasVersionOverride = false;
    std::uint64_t m_revision = 0;
    DirectX::XMFLOAT2 m_screenSize = {};
    const TitleUiSettings::Data& Settings() const;
    void ApplySettings();
    void ApplyView();
public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    void Setup(const TitleUiSettingsAsset* settings);
    void DestroyWidgets();
    void SetSelectedMenu(TitleUi::MenuItem item, bool animate = true);
    void SetMenuVisible(bool visible);
    void SetVisible(bool visible);
    void SetExitPopupVisible(bool visible);
    void SetExitConfirmationSelection(bool yesSelected, bool animate = true);
    void SetHighScore(int score);
    void SetVersionText(const std::string& version);
};
