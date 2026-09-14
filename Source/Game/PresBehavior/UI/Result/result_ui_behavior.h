#pragma once

#include "Engine/Component/behavior_component.h"

class ResultUiBehavior : public BehaviorComponent {
public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};
