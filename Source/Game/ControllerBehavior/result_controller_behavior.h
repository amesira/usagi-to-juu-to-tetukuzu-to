#pragma once

#include "Engine/Component/behavior_component.h"

class ResultControllerBehavior : public BehaviorComponent {
public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};
