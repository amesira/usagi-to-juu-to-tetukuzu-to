#pragma once

#include "Engine/Component/behavior_component.h"
#include "result_controller_settings_asset.h"
#include <cstdint>

class ResultControllerBehavior : public BehaviorComponent {
private:
    const ResultControllerSettingsAsset* m_settings = nullptr;
    const ResultControllerSettings::Data& Settings() const;
    std::uint64_t m_settingsAssetRevision = 0;

    class TransformComponent* m_resultTextTransform = nullptr;
    class ResultUiBehavior* m_resultUi = nullptr;
    bool m_leaving = false;

public:
    void Setup(const ResultControllerSettingsAsset* settings) { m_settings = settings; }
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
};
