//---------------------------------------------------
// File  ：_/UI/Player/player_ui_view.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・PlayerUiの見た目制御を担当するクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Game/PresBehavior/UI/ui_handle.h"
#include "player_ui_context.h"

class PlayerUiView {
private:

public:
    void SetWidgetGroupAlpha(
        PlayerUi::PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        float alpha);
    void SetWidgetGroupPosition(
        PlayerUi::PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        const DirectX::XMFLOAT2& position);

};
