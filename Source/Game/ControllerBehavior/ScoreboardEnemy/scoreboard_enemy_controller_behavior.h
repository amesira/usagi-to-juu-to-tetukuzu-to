#pragma once
#include "Engine/Component/behavior_component.h"

class ScoreboardEnemySettingsAsset;

class ScoreboardEnemyControllerBehavior : public BehaviorComponent {
    const ScoreboardEnemySettingsAsset* m_settings = nullptr;
    bool m_created = false;
public:
    void Setup(const ScoreboardEnemySettingsAsset* settings) { m_settings = settings; }
    void Start() override {}
    void Update() override;
    void DrawComponentInspector() override;
private:
    void CreateEnemies();
};
