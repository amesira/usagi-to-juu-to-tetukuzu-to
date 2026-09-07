//===================================================
// File  ：_/PresBehavior/UI/Player/player_ui_behavior.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Device/mi_fps.h"

void PlayerUiBehavior::Start()
{
    m_context.owner = this;
    m_context.scene = GetOwner()->GetScene();

    m_context.view = &m_view;
    m_context.presentation = &m_presentation;
    m_presentation.Initialize();
}

void PlayerUiBehavior::Update()
{
    m_presentation.Update(m_context, FPS_GetUnscaledDeltaTime());
}

void PlayerUiBehavior::DrawComponentInspector()
{

}
