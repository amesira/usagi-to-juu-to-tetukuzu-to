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
#include "Engine/Core/GamePlay/tween_task.h"
#include "ui_shake_task.h"
#include <array>

class PlayerUiView;

class PlayerUiPresentation {
private:
    struct WidgetGroupTasks {
        FloatTweenTask fade;
        PlayerUi::ShakeTask shake;
    };
    std::array<WidgetGroupTasks, static_cast<size_t>(PlayerUi::WidgetGroupID::Max)> m_groupTasks;

public:
    void Initialize();
    void Update(PlayerUiContext& context, float deltaTime);

    void FadeWidgetGroup(
        PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        float targetAlpha, 
        float duration);

    // === 演出のキャンセル ===
    void CancelFadeWidgetGroup(PlayerUiContext& context, PlayerUi::WidgetGroupID groupID);
    void CancelShakeWidgetGroup(PlayerUiContext& context, PlayerUi::WidgetGroupID groupID);
    void CancelAll(PlayerUiContext& context);
    void ShakeWidgetGroup(
        PlayerUiContext& context, 
        PlayerUi::WidgetGroupID groupID, 
        float intensity, 
        float duration);

};
