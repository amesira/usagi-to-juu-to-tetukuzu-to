#pragma once
#include "title_ui_settings_asset.h"
#include "Game/PresBehavior/UI/ui_handle.h"
#include <array>
#include <vector>

namespace TitleUi {
    enum class MenuItem { Practice, StartGame, Exit };
    struct Element {
        UiHandle handle;
        UiLayoutSettings::WidgetTransform layout;
    };
    struct Group {
        std::vector<Element> elements;
        TitleUiSettings::GroupSettings settings;
        bool visible = true;
        void Destroy();
    };
}

class TitleUiMenu {
public:
    TitleUi::Group group;
    void Initialize(IScene* scene);
    void ApplySettings(const TitleUiSettings::Data& settings, TitleUi::MenuItem selected);
    DirectX::XMFLOAT2 GetSelectionTarget(TitleUi::MenuItem selected) const;
};
class TitleUiExitPopup {
public:
    TitleUi::Group group;
    UiHandle dimmer;
    void Initialize(IScene* scene);
    void ApplySettings(const TitleUiSettings::Data& settings, bool yesSelected);
    DirectX::XMFLOAT2 GetSelectionTarget(bool yesSelected) const;
    void Destroy();
};
class TitleUiVersion {
public:
    TitleUi::Group group;
    void Initialize(IScene* scene);
    void ApplySettings(const TitleUiSettings::Data& settings, const std::string& value);
};
class TitleUiHighScore {
public:
    TitleUi::Group group;
    void Initialize(IScene* scene);
    void ApplySettings(const TitleUiSettings::Data& settings, int score);
};
