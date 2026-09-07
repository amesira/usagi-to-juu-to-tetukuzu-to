//---------------------------------------------------
// File  ：_/PresBehavior/UI/Player/player_ui_behavior.h
// Date  ：2026/09/07
// Author：Miu Kitamura
// 
// ・PlayerUiを制御するためのBehavior
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"

#include "Game/PresBehavior/UI/ui_handle.h"
#include "player_ui_context.h"
#include "player_ui_presentation.h"
#include "player_ui_view.h"

class PlayerUiBehavior : public BehaviorComponent {
private:
    PlayerUiContext m_context;

    PlayerUiView m_view;
    PlayerUiPresentation m_presentation;

public:
    PlayerUiBehavior() = default;
    ~PlayerUiBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

};