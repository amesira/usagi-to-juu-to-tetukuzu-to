//===================================================
// File  ：_/PresBehavior/UI/Player/player_ui_behavior.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "player_ui_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

void PlayerUiBehavior::Start()
{
    m_context.owner = this;
    m_context.scene = GetOwner()->GetScene();

    m_context.view = &m_view;
    m_context.presentation = &m_presentation;

}

void PlayerUiBehavior::Update()
{

}

void PlayerUiBehavior::DrawComponentInspector()
{

}