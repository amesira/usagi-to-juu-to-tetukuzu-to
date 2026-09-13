#pragma once
#include "Engine/Component/behavior_component.h"
#include "stage_decoration_settings_asset.h"
#include <cstdint>

class StageDecorationControllerBehavior : public BehaviorComponent {
    struct GeneratedObject {
        unsigned int id;
        std::string name;
        DirectX::XMFLOAT4 initialRotation;
        DirectX::XMFLOAT3 axis;
        float speed;
        float angle = 0;
    };
    const StageDecorationSettingsAsset* m_settingsAsset = nullptr;
    std::vector<GeneratedObject> m_objects;
    std::uint64_t m_revision = 0;
    unsigned int m_serial = 0;
    bool m_rebuildRequested = false;
    std::string m_status;
public:
    ~StageDecorationControllerBehavior() override;
    void Setup(const StageDecorationSettingsAsset* asset) { m_settingsAsset = asset; m_rebuildRequested = true; }
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;
    void RequestRebuild() { m_rebuildRequested = true; }
    void DestroyGeneratedObjects();
private:
    void Rebuild();
};
