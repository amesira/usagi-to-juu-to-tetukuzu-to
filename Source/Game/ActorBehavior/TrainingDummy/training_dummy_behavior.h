//---------------------------------------------------
// File  ：_/TrainingDummy/training_dummy_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・練習用のかかしの振る舞いをまとめるBehavior
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"
#include "Game/ActorBehavior/Base/HitReceiver/hit_receiver_context.h"

#include "training_dummy_effects.h"
#include "training_dummy_motions.h"

class TrainingDummyBehavior : public BehaviorComponent {
private:
    class TransformComponent* m_transform = nullptr;
    class RigidbodyComponent* m_rigidbody = nullptr;

    class HitReceiverBehavior* m_hitReceiverBehavior = nullptr;
    class HealthBehavior* m_healthBehavior = nullptr;

    TrainingDummyEffects m_effects;
    TrainingDummyMotions m_motions;

public:
    TrainingDummyBehavior() = default;
    ~TrainingDummyBehavior() override = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    void OnHitReceived(const HitReceiver::HitData& hitData, const HitReceiver::HitResult& hitResult);

};

