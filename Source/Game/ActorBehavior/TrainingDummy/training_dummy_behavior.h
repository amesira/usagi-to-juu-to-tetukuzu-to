//---------------------------------------------------
// File  ：_/TrainingDummy/training_dummy_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・練習用のかかしの振る舞いをまとめるBehavior
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"

class TrainingDummyBehavior : public BehaviorComponent {
private:
    class TransformComponent* m_transform = nullptr;

public:
    TrainingDummyBehavior() = default;
    ~TrainingDummyBehavior() override = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};

