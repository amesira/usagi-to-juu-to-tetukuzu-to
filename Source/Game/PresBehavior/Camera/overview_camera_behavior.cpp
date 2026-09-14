#include "overview_camera_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/camera_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Device/mi_fps.h"
#include "External/ImGui/imgui.h"

const OverviewCameraSettings::Data& OverviewCameraBehavior::Settings() const {
    static const OverviewCameraSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}
void OverviewCameraBehavior::Start() {
    m_transform = GetOwner()->GetComponent<TransformComponent>();
    m_camera = GetOwner()->GetComponent<CameraComponent>();
    if (auto* tps = GetOwner()->GetScene()->GetGameObjectByName("MainCamera")) {
        m_tpsTransform = tps->GetComponent<TransformComponent>();
        m_tpsCamera = tps->GetComponent<CameraComponent>();
    }
    ReturnToOverview();
}
void OverviewCameraBehavior::ReturnToOverview() {
    m_mode = Mode::Overhead;
    if (!m_transform || !m_camera) return;
    const auto& settings = Settings();
    m_transform->SetPosition(settings.position);
    m_camera->SetAtPosition(settings.lookAt);
    m_camera->SetFov(settings.fovDegrees);
}
bool OverviewCameraBehavior::BeginTpsFollow() {
    if (!m_transform || !m_camera || !m_tpsTransform || !m_tpsCamera || m_mode != Mode::Overhead) return false;
    m_blend.Start({m_transform->GetPosition(),m_camera->GetAtPosition(),m_camera->GetFov()}, Settings().blendDuration);
    m_mode = Mode::BlendingToTps;
    return true;
}
void OverviewCameraBehavior::LateUpdate() {
    if (!m_transform || !m_camera) return;
    if (m_mode == Mode::Overhead) { ReturnToOverview(); return; }
    if (!m_tpsTransform || !m_tpsCamera) return;
    OverviewCameraPose pose = {m_tpsTransform->GetPosition(), m_tpsCamera->GetAtPosition(), m_tpsCamera->GetFov()};
    if (m_mode == Mode::BlendingToTps) {
        pose = m_blend.Update(pose, FPS_GetUnscaledDeltaTime());
        if (m_blend.IsComplete()) m_mode = Mode::FollowingTps;
    }
    m_transform->SetPosition(pose.position);
    m_camera->SetAtPosition(pose.lookAt);
    m_camera->SetFov(pose.fov);
    m_camera->SetAspect(m_tpsCamera->GetAspect());
    m_camera->SetNearClip(m_tpsCamera->GetNearClip());
    m_camera->SetFarClip(m_tpsCamera->GetFarClip());
    m_camera->SetUpVector(m_tpsCamera->GetUpVector());
}
void OverviewCameraBehavior::DrawComponentInspector() {
    const char* modes[] = {"Overhead", "Blending to TPS", "Following TPS"};
    ImGui::Text("Overview Camera: %s", modes[static_cast<int>(m_mode)]);
}
