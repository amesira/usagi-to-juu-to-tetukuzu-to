#pragma once
#include "Engine/Component/behavior_component.h"
#include "overview_camera_settings_asset.h"
#include "overview_camera_blend.h"

class OverviewCameraBehavior : public BehaviorComponent {
public:
    enum class Mode { Overhead, BlendingToTps, FollowingTps };
    void Setup(const OverviewCameraSettingsAsset* settings) { m_settings = settings; }
    void Start() override;
    void Update() override {}
    void LateUpdate() override;
    void DrawComponentInspector() override;
    bool BeginTpsFollow();
    void ReturnToOverview();
    Mode GetMode() const { return m_mode; }
private:
    const OverviewCameraSettingsAsset* m_settings = nullptr;
    class TransformComponent* m_transform = nullptr;
    class CameraComponent* m_camera = nullptr;
    class TransformComponent* m_tpsTransform = nullptr;
    class CameraComponent* m_tpsCamera = nullptr;
    OverviewCameraBlend m_blend;
    Mode m_mode = Mode::Overhead;
    const OverviewCameraSettings::Data& Settings() const;
};
