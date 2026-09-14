#pragma once

#include "Engine/Component/behavior_component.h"
#include "result_controller_settings_asset.h"

class ResultControllerBehavior : public BehaviorComponent {
private:
    const ResultControllerSettingsAsset* m_settings = nullptr;
    const ResultControllerSettings::Data& Settings() const;
public:
    void Setup(const ResultControllerSettingsAsset* settings) { m_settings = settings; }
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};
