//---------------------------------------------------
// File  ：_/UI/Player/player_ui_presentation.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・PlayerUiの演出を担当するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "player_ui_context.h"

class PlayerUiView;

class PlayerUiPresentation {
private:

public:
    void Initialize();
    void Update(PlayerUiContext& context, float deltaTime);

    void FadeWidgetGroup(
        PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        float targetAlpha, 
        float duration);
    void ShakeWidgetGroup(
        PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        float intensity, 
        float duration);

};
