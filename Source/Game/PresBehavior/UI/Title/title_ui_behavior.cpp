#include "title_ui_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Device/direct3d.h"
#include "Engine/Device/mi_fps.h"
#include "External/ImGui/imgui.h"

const TitleUiSettings::Data& TitleUiBehavior::Settings() const {
    static const TitleUiSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}
void TitleUiBehavior::Setup(const TitleUiSettingsAsset* settings) { m_settings = settings; m_dirty = true; }
void TitleUiBehavior::Start() {
    if (m_created || !GetOwner() || !GetOwner()->GetScene()) return;
    auto* scene = GetOwner()->GetScene();
    m_menu.Initialize(scene); m_popup.Initialize(scene);
    m_version.Initialize(scene); m_highScore.Initialize(scene);
    m_created = true; m_dirty = true;
    Update();
}
void TitleUiBehavior::Update() {
    if (!m_created) return;
    const DirectX::XMFLOAT2 size = {static_cast<float>(Direct3D_GetBackBufferWidth()), static_cast<float>(Direct3D_GetBackBufferHeight())};
    const auto revision = m_settings ? m_settings->GetRevision() : 0;
    if (m_dirty || revision != m_revision || size.x != m_screenSize.x || size.y != m_screenSize.y) {
        m_screenSize = size; m_revision = revision;
        ApplySettings(); m_dirty = false;
    }
    m_presentation.Update(FPS_GetUnscaledDeltaTime());
    ApplyView();
}
void TitleUiBehavior::ApplySettings() {
    const auto& s = Settings();
    m_menu.ApplySettings(s, m_selected); m_popup.ApplySettings(s, m_yesSelected);
    m_version.ApplySettings(s, m_hasVersionOverride ? m_versionOverride : s.version.value);
    m_highScore.ApplySettings(s, m_score);
    m_presentation.menuSelection.MoveTo(m_menu.GetSelectionTarget(m_selected), 0);
    m_presentation.popupSelection.MoveTo(m_popup.GetSelectionTarget(m_yesSelected), 0);
}
void TitleUiBehavior::ApplyView() {
    m_menu.group.visible = m_menuVisible;
    m_highScore.group.visible = m_menuVisible;
    m_popup.group.visible = m_popupVisible;
    const auto& s = Settings();
    m_view.ApplyGroup(m_menu.group, s, m_screenSize, 0, m_presentation.menuSelection.GetPosition());
    m_view.ApplyGroup(m_popup.group, s, m_screenSize, 1, m_presentation.popupSelection.GetPosition());
    m_view.ApplyGroup(m_version.group, s, m_screenSize);
    m_view.ApplyGroup(m_highScore.group, s, m_screenSize);
    m_view.ApplyDimmer(m_popup, m_screenSize);
}
void TitleUiBehavior::SetSelectedMenu(TitleUi::MenuItem item, bool animate) {
    if (item < TitleUi::MenuItem::Practice || item > TitleUi::MenuItem::Exit) return;
    if (item == m_selected) return;
    m_selected = item;
    if (!m_created) return;
    m_menu.ApplySettings(Settings(), item);
    m_presentation.menuSelection.MoveTo(m_menu.GetSelectionTarget(item), animate ? Settings().selectionMoveDuration : 0);
    ApplyView();
}
void TitleUiBehavior::SetMenuVisible(bool visible) { m_menuVisible = visible; if (m_created) ApplyView(); }
void TitleUiBehavior::SetExitPopupVisible(bool visible) {
    m_popupVisible = visible;
    if (visible) SetExitConfirmationSelection(false, false);
    if (m_created) ApplyView();
}
void TitleUiBehavior::SetExitConfirmationSelection(bool yesSelected, bool animate) {
    m_yesSelected = yesSelected;
    if (!m_created) return;
    m_popup.ApplySettings(Settings(), yesSelected);
    m_presentation.popupSelection.MoveTo(m_popup.GetSelectionTarget(yesSelected), animate ? Settings().selectionMoveDuration : 0);
    ApplyView();
}
void TitleUiBehavior::SetHighScore(int score) { m_score = (std::max)(score, 0); m_dirty = true; }
void TitleUiBehavior::SetVersionText(const std::string& version) { m_versionOverride = version; m_hasVersionOverride = true; m_dirty = true; }
void TitleUiBehavior::DestroyWidgets() {
    if (!m_created) return;
    m_menu.group.Destroy(); m_popup.Destroy(); m_version.group.Destroy(); m_highScore.group.Destroy();
    m_presentation = {}; m_created = false; m_dirty = true;
}
void TitleUiBehavior::DrawComponentInspector() {
    ImGui::PushID(this);
    if (!m_created) ImGui::TextUnformatted("Title UI is created when play starts.");
    else {
        int selected = static_cast<int>(m_selected);
        if (ImGui::Combo("Menu preview", &selected, "Practice\0Start Game\0Exit\0")) SetSelectedMenu(static_cast<TitleUi::MenuItem>(selected));
        bool menu = m_menuVisible, popup = m_popupVisible, yes = m_yesSelected;
        if (ImGui::Checkbox("Show menu", &menu)) SetMenuVisible(menu);
        if (ImGui::Checkbox("Show exit popup", &popup)) SetExitPopupVisible(popup);
        if (ImGui::Checkbox("Select yes", &yes)) SetExitConfirmationSelection(yes);
        if (ImGui::Button("Reapply layout")) m_dirty = true;
    }
    ImGui::PopID();
}
