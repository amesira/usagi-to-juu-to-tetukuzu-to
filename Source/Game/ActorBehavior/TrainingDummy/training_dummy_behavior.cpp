//===================================================
// File  ：_/TrainingDummy/training_dummy_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "training_dummy_behavior.h"

#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"

#include "Engine/Editor/BaseEditor/inspector_view_window.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/rigidbody_component.h"

#include "Game/ActorBehavior/Base/health_behavior.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_behavior.h"

void TrainingDummyBehavior::Start()
{
    m_transform = GetOwner()->GetComponent<TransformComponent>();
    m_healthBehavior = GetOwner()->GetComponent<HealthBehavior>();
    m_hitReceiverBehavior = GetOwner()->GetComponent<HitReceiverBehavior>();

    // ノックバック移動方法を指定
    if (m_hitReceiverBehavior) {
        m_hitReceiverBehavior->KnockbackReceiver()->SetDefaultMovementSource({
            KnockbackMovementMode::SetRigidbodyVelocity,
            true,
            -9.81f
            });
    }
}

void TrainingDummyBehavior::Update()
{
    // 練習用のかかしの振る舞いをここに実装する

    // ダメージを受けてから一定時間経過で回復

}

void TrainingDummyBehavior::DrawComponentInspector()
{
    if (InspectorViewWindow::BeginComponentSection(this, "TrainingDummyBehavior"))
    {
        ImGui::Text("This is a training dummy behavior.");
    }

    InspectorViewWindow::EndComponentSection();
}