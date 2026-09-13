#include "title_ui_widgets.h"
#include "Game/Factory/ui_factory.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/engine_service_locator.h"

namespace {
    void Add(TitleUi::Group& group, UiHandle handle, const char* name, float layer) {
        group.elements.push_back({handle, {}});
        if (auto* object = handle.GetGameObject()) object->SetName(name);
        if (auto* rect = handle.GetRectTransform()) rect->SetPosition({0, 0, layer});
    }
    void Text(TitleUi::Element& element, const std::string& value,
        const TitleUiSettings::Data& settings, bool selected = false) {
        if (auto* text = element.handle.GetText()) {
            text->SetText(value);
            text->SetFontSize(settings.fontSize);
        }
        element.handle.SetColor(selected ? settings.selectedTextColor : settings.textColor);
    }
    void ApplyImage(UiHandle handle, const std::string& path, DirectX::XMFLOAT3 color, float opacity) {
        if (auto* image = handle.GetImage()) {
            if (TEXTURE_REPOSITORY) {
                // Missing custom images fall back to the existing solid white texture.
                auto* texture = TEXTURE_REPOSITORY->GetTextureResource(path);
                if (!texture) texture = TEXTURE_REPOSITORY->GetTextureResource("asset/Texture/white.bmp");
                if (texture) image->SetTextureResource(texture);
            }
        }
        handle.SetColor(color);
        handle.SetAlpha(opacity);
    }
}
void TitleUi::Group::Destroy() {
    for (auto& element : elements) element.handle.Destroy();
    elements.clear();
}
void TitleUiMenu::Initialize(IScene* scene) {
    if (!group.elements.empty()) return;
    Add(group, UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp"), "TitleUi.Menu.Selection", 100);
    for (const char* name : {"TitleUi.Menu.Practice", "TitleUi.Menu.StartGame", "TitleUi.Menu.Exit"})
        Add(group, UiFactory::CreateUiTextHandle(scene, u8""), name, 101);
}
void TitleUiMenu::ApplySettings(const TitleUiSettings::Data& s, TitleUi::MenuItem selected) {
    if (group.elements.size() != 4) return;
    group.settings = s.menu.group;
    group.elements[0].layout = s.menu.selection;
    group.elements[1].layout = s.menu.practice;
    group.elements[2].layout = s.menu.startGame;
    group.elements[3].layout = s.menu.exit;
    ApplyImage(group.elements[0].handle, s.menu.selectionImagePath, s.selectionColor, s.selectionOpacity);
    const std::string* labels[] = {&s.menu.practiceText, &s.menu.startGameText, &s.menu.exitText};
    for (size_t i = 0; i < 3; ++i) Text(group.elements[i + 1], *labels[i], s, i == static_cast<size_t>(selected));
}
DirectX::XMFLOAT2 TitleUiMenu::GetSelectionTarget(TitleUi::MenuItem item) const {
    const auto row = group.elements[1 + static_cast<size_t>(item)].layout.position;
    const auto offset = group.elements[0].layout.position;
    return {row.x + offset.x, row.y + offset.y};
}
void TitleUiExitPopup::Initialize(IScene* scene) {
    if (!group.elements.empty()) return;
    dimmer = UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp");
    if (auto* object = dimmer.GetGameObject()) object->SetName("TitleUi.Popup.Dimmer");
    if (auto* rect = dimmer.GetRectTransform()) rect->SetPosition({0, 0, 200});
    Add(group, UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp"), "TitleUi.Popup.Panel", 201);
    Add(group, UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp"), "TitleUi.Popup.Selection", 202);
    for (const char* name : {"TitleUi.Popup.Message", "TitleUi.Popup.Yes", "TitleUi.Popup.No"})
        Add(group, UiFactory::CreateUiTextHandle(scene, u8""), name, 203);
}
void TitleUiExitPopup::ApplySettings(const TitleUiSettings::Data& s, bool yesSelected) {
    if (group.elements.size() != 5) return;
    group.settings = s.popup.group;
    group.elements[0].layout = s.popup.panel;
    group.elements[1].layout = s.popup.selection;
    group.elements[2].layout = s.popup.message;
    group.elements[3].layout = s.popup.yes;
    group.elements[4].layout = s.popup.no;
    ApplyImage(group.elements[0].handle, s.popup.panelImagePath, s.panelColor, 1);
    ApplyImage(group.elements[1].handle, s.popup.selectionImagePath, s.selectionColor, s.selectionOpacity);
    Text(group.elements[2], s.popup.messageText, s);
    Text(group.elements[3], s.popup.yesText, s, yesSelected);
    Text(group.elements[4], s.popup.noText, s, !yesSelected);
    dimmer.SetColor({0, 0, 0});
    dimmer.SetAlpha(s.popup.dimmerOpacity);
}
DirectX::XMFLOAT2 TitleUiExitPopup::GetSelectionTarget(bool yesSelected) const {
    const auto row = group.elements[yesSelected ? 3 : 4].layout.position;
    const auto offset = group.elements[1].layout.position;
    return {row.x + offset.x, row.y + offset.y};
}
void TitleUiExitPopup::Destroy() { group.Destroy(); dimmer.Destroy(); dimmer = {}; }
void TitleUiVersion::Initialize(IScene* scene) {
    if (group.elements.empty()) Add(group, UiFactory::CreateUiTextHandle(scene, u8""), "TitleUi.Version", 110);
}
void TitleUiVersion::ApplySettings(const TitleUiSettings::Data& s, const std::string& value) {
    group.settings = s.version.group;
    group.elements[0].layout = s.version.text;
    Text(group.elements[0], value, s);
}
void TitleUiHighScore::Initialize(IScene* scene) {
    if (group.elements.empty()) Add(group, UiFactory::CreateUiTextHandle(scene, u8""), "TitleUi.HighScore", 110);
}
void TitleUiHighScore::ApplySettings(const TitleUiSettings::Data& s, int score) {
    group.settings = s.highScore.group;
    group.elements[0].layout = s.highScore.text;
    Text(group.elements[0], s.highScore.value + "  " + std::to_string(score), s);
}
