#ifndef TITLE_CONTROLLER_BEHAVIOR_H
#define TITLE_CONTROLLER_BEHAVIOR_H

#include "Engine/Component/behavior_component.h"

// タイトルの状態・メニュー進行を担当する。
class TitleControllerBehavior : public BehaviorComponent {
public:
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};

#endif // TITLE_CONTROLLER_BEHAVIOR_H
