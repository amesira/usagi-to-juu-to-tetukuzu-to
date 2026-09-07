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
public:
    void SetWidgetGroupAlpha(
        PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        float alpha);
    void SetWidgetGroupPosition(
        PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        const DirectX::XMFLOAT2& position);

    // 基準位置を変えずに演出オフセットだけを反映する。
    void SetWidgetGroupShakeOffset(PlayerUiContext& context,
        PlayerUi::WidgetGroupID groupID, const DirectX::XMFLOAT2& offset);

private:
    void ApplyWidgetGroupPosition(PlayerUi::WidgetGroup& group);

};
