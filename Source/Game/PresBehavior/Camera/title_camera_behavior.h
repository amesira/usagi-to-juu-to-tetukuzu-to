#pragma once
#include "Engine/Component/behavior_component.h"
#include "title_camera_settings_asset.h"
#include "title_camera_blend.h"

class TitleCameraBehavior : public BehaviorComponent {
public:
    enum class Mode { Overhead, BlendingToTps, FollowingTps };
    void Setup(const TitleCameraSettingsAsset* settings) { m_settings = settings; }
    void Start() override;
    void Update() override {}
    void LateUpdate() override;
    void DrawComponentInspector() override;
    bool BeginPractice();
    void ReturnToTitle();
    Mode GetMode() const { return m_mode; }
private:
    const TitleCameraSettingsAsset* m_settings = nullptr;
    class TransformComponent* m_transform = nullptr;
    class CameraComponent* m_camera = nullptr;
    class TransformComponent* m_tpsTransform = nullptr;
    class CameraComponent* m_tpsCamera = nullptr;
    TitleCameraBlend m_blend;
    Mode m_mode = Mode::Overhead;
    const TitleCameraSettings::Data& Settings() const;
};
