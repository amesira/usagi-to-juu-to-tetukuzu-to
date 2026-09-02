//===================================================
// File  ：_/TrainingDummy/training_dummy_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "training_dummy_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"
#include "Engine/Device/mi_fps.h"

#include "Engine/Editor/BaseEditor/inspector_view_window.h"

void TrainingDummyBehavior::Start()
{
    m_transform = GetOwner()->GetComponent<TransformComponent>();
}

void TrainingDummyBehavior::Update()
{
    // 練習用のかかしの振る舞いをここに実装する
}

void TrainingDummyBehavior::DrawComponentInspector()
{
    if (InspectorViewWindow::BeginComponentSection(this, "TrainingDummyBehavior"))
    {
        ImGui::Text("This is a training dummy behavior.");
    }

    InspectorViewWindow::EndComponentSection();
}