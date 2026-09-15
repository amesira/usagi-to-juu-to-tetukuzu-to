#pragma once
#include "Engine/Component/behavior_component.h"

class ParticleSystemComponent;

class OneShotParticleBehavior : public BehaviorComponent {
    ParticleSystemComponent* m_particles = nullptr;
public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};
