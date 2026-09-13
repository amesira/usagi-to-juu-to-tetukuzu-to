#pragma once
#include "title_ui_widgets.h"

class TitleUiView {
public:
    void ApplyGroup(TitleUi::Group& group, const TitleUiSettings::Data& settings,
        DirectX::XMFLOAT2 screenSize, int selectionIndex = -1,
        DirectX::XMFLOAT2 selectionPosition = {});
    void ApplyDimmer(TitleUiExitPopup& popup, DirectX::XMFLOAT2 screenSize);
};
