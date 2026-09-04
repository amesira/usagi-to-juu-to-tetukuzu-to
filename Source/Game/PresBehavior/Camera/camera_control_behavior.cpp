//===================================================
// camera_control_behavior.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/04/08
//===================================================
#include "camera_control_behavior.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include <algorithm>

#include "Utility/mi_math.h"
#include "Utility/mi_signal.h"

#include "Engine/Device/mi_fps.h"
#include "Engine/Device/keyboard.h"
#include "Engine/Device/mouse.h"
#include "Engine/engine_service_locator.h"

#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/camera_component.h"
#include "Engine/Component/rigidbody_component.h"

using namespace CameraEffectTaskHelper;

namespace {
    constexpr Keyboard_Keys CAMERA_INPUT_DISABLE_KEY = KK_F1;
}

void CameraControlBehavior::Start()
{
    m_context.owner = this;
    m_context.transform = GetOwner()->GetComponent<TransformComponent>();
    m_context.camera = GetOwner()->GetComponent<CameraComponent>();

    if (m_context.settingsAsset == nullptr) {
        m_context.settingsAsset = DATA_LOADER->GetAsset<CameraSettingsAsset>(
            "asset/Data/camera_settings.data.json", true);
    }
    if (m_context.settingsAsset == nullptr) return;

    m_context.runtimeState.Initialize(m_context.settings());
    ApplyCurrentSettings();

    // 仮：DataAssetのロード完了時に呼ばれるコールバックを登録
    m_settingsReloadCallback = [this]() { ApplyCurrentSettings(); };

    IScene* scene = GetOwner()->GetScene();
    GameObject* target = scene->GetGameObjectByName("Player");
    if (target) {
        m_context.references.targetTransform = target->GetComponent<TransformComponent>();
    }

    m_context.cameraEffect.Initialize(m_context);
}

/// @brief カメラの設定を切り替える
void CameraControlBehavior::SetSettingsAsset(const CameraSettingsAsset* settingsAsset)
{
    if (!settingsAsset || m_context.settingsAsset == settingsAsset) return;

    m_context.settingsAsset = settingsAsset;
    m_lastSettingsRevision = settingsAsset->GetRevision();
    ApplyCurrentSettings();
}

/// @brief 現在のカメラ設定を適用する
void CameraControlBehavior::ApplyCurrentSettings()
{
    if (!m_context.settingsAsset) return;

    CameraRuntimeState& state = m_context.runtimeState;
    const CameraSettings::Data& settings = m_context.settings();

    state.followDistance = settings.followDistance;
    state.compositionWorldOffset = settings.compositionWorldOffset;
    state.compositionCameraLocalOffset = settings.compositionCameraLocalOffset;
    state.fov = settings.fov;
    state.targetPitch = MiMath::Clamp(state.targetPitch, settings.minPitch, settings.maxPitch);
    state.pitch = MiMath::Clamp(state.pitch, settings.minPitch, settings.maxPitch);
    state.ResetSmoothDampVelocity();

    if (m_context.camera) {
        m_context.camera->SetFov(state.fov);
    }
}

void CameraControlBehavior::Update()
{
    if (m_context.settingsAsset == nullptr) return;
    float deltaTime = FPS_GetUnscaledDeltaTime();

    // === DataAssetのリロード検知 ===
    if (m_lastSettingsRevision != m_context.settingsAsset->GetRevision()) {
        m_lastSettingsRevision = m_context.settingsAsset->GetRevision();

        // 設定が変更された場合の処理
        m_settingsReloadCallback();
    }

    // カメラエフェクトタスクの更新
    m_context.cameraEffect.UpdateCameraEffectTasks(m_context, deltaTime);

    // 入力の有効・無効を切り替える操作を処理
    UpdateCameraInputActivation();

    CameraRuntimeState& state = m_context.runtimeState;
    const CameraSettings::Data& settings = m_context.settings();

    // 1.カメラ回転処理
    if (state.isInputEnabled) {
        UpdateTargetYawPitchFromInput(deltaTime);
    }
    state.targetPitch = MiMath::Clamp(state.targetPitch, settings.minPitch, settings.maxPitch);

    // カメラの回転をスムーズに追従
    state.pitch = MiMath::SmoothDamp(state.pitch, state.targetPitch, state.pitchVelocity, settings.rotationSmoothTime, deltaTime);
    state.yaw = MiMath::SmoothDamp(state.yaw, state.targetYaw, state.yawVelocity, settings.rotationSmoothTime, deltaTime);

    // 2.ForwardとRightの計算
    BuildCameraBasis(state.cameraForward, state.cameraRight);

    // 3.追従アンカー位置を更新する
    XMFLOAT3 targetFollowAnchorPosition = CalculateTargetFollowAnchorPosition();
    UpdateFollowAnchorPosition(targetFollowAnchorPosition, deltaTime);

    // 絶対的なオフセットをここで加算（補間を行ないたくないパラメータ）
    XMFLOAT3 lookAtPosition = MiMath::Add(
        state.followAnchorPosition,
        CalculateCompositionOffset()
    );

    // === カメラシェイクの適用 ===
    if (m_context.cameraEffect.IsCameraShaking()) {
        lookAtPosition = MiMath::Add(lookAtPosition, m_context.cameraEffect.GetShakeOffset());
    }

    // 4.CameraEyePositionを更新する
    XMFLOAT3 cameraPosition = MiMath::Add(
        lookAtPosition,
        MiMath::Multiply(state.cameraForward, -EvaluateCameraDistance())
    );

    // 5.適用
    m_context.transform->SetPosition(cameraPosition);
    m_context.camera->SetAtPosition(lookAtPosition);
}

// ImGuiを使ったインスペクタの描画
void CameraControlBehavior::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Camera Control Behavior"))
    {
        CameraRuntimeState& state = m_context.runtimeState;

        ImGui::Text("Camera Input: %s", state.isInputEnabled ? "Enabled" : "Disabled");

        float followDistance = state.followDistance;
        if (ImGui::SliderFloat("Follow Distance", &followDistance, 5.0f, 30.0f)) {
            state.followDistance = followDistance;
        }
        if (m_context.settingsAsset) {
            ImGui::Text("LookAt Height: %.2f", m_context.settings().lookAtHeight);
        }

        float targetPitchDegrees = XMConvertToDegrees(state.targetPitch);
        if (ImGui::SliderFloat("Target Pitch", &targetPitchDegrees, -90.0f, 90.0f)) {
            state.targetPitch = XMConvertToRadians(targetPitchDegrees);
        }
        float targetYawDegrees = XMConvertToDegrees(state.targetYaw);
        if (ImGui::SliderFloat("Target Yaw", &targetYawDegrees, -180.0f, 180.0f)) {
            state.targetYaw = XMConvertToRadians(targetYawDegrees);
        }

        if (ImGui::TreeNode("Camera Distance")) {
            static float distance = 10.0f;
            static float duration = 0.2f;
            static float holdDuration = 0.2f;

            ImGui::DragFloat("Distance", &distance, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Duration", &duration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Hold Duration", &holdDuration, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Change Distance")) {
                ChangeCameraEffect(this, CameraEffect::EffectTaskTarget::Distance, distance, duration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Temporary")) {
                ChangeCameraEffectTemporary(this, CameraEffect::EffectTaskTarget::Distance, distance, duration, holdDuration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                ResetCameraEffect(this, CameraEffect::EffectTaskTarget::Distance, duration);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Composition Camera Local Offset")) {
            static XMFLOAT3 offset = { 0.8f, 0.2f, 0.0f };
            static float duration = 0.2f;
            static float holdDuration = 0.2f;

            ImGui::DragFloat3("Camera Local Offset", &offset.x, 0.05f, -10.0f, 10.0f);
            ImGui::DragFloat("Duration", &duration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Hold Duration", &holdDuration, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Change Local Offset")) {
                ChangeCameraEffect(this, CameraEffect::EffectTaskTarget::CompositionCameraLocalOffset, offset, duration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Temporary")) {
                ChangeCameraEffectTemporary(this, CameraEffect::EffectTaskTarget::CompositionCameraLocalOffset, offset, duration, holdDuration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                ResetCameraEffect(this, CameraEffect::EffectTaskTarget::CompositionCameraLocalOffset, duration);
            }

            ImGui::TreePop();
        }
    }

    BehaviorDetailView::EndSection();
}

// ----- CameraEffect関連の操作 -----

/// @brief カメラエフェクトタスクをリクエストする
void CameraControlBehavior::RequestCameraEffectTask(CameraEffect::EffectTaskTarget target, const CameraEffect::RequestEffectTaskInfo& requestInfo)
{
    m_context.cameraEffect.RequestEffectTask(m_context, target, requestInfo);
}

/// @brief カメラシェイクを再生する
void CameraControlBehavior::PlayCameraShake(float duration, float magnitude)
{
    m_context.cameraEffect.PlayCameraShake(m_context, duration, magnitude);
}

// ----- private method -----

/// @brief 入力の有効・無効を切り替える操作を処理
void CameraControlBehavior::UpdateCameraInputActivation()
{
    if (Keyboard_IsKeyDownTrigger(CAMERA_INPUT_DISABLE_KEY)) {
        if (m_context.runtimeState.isInputEnabled) {
            m_context.runtimeState.isInputEnabled = false;
        }
        else {
            m_context.runtimeState.isInputEnabled = true;
        }
    }
}

/// @brief カメラ回転のターゲット値の入力による更新
void CameraControlBehavior::UpdateTargetYawPitchFromInput(float deltaTime)
{
    CameraRuntimeState& state = m_context.runtimeState;
    const CameraSettings::Data& settings = m_context.settings();

    // マウス入力から回転のターゲット値を計算
    float mouseX = Mouse_GetPositionX() - Mouse_GetOldPositionX();
    float mouseY = Mouse_GetPositionY() - Mouse_GetOldPositionY();

    state.targetYaw += mouseX * settings.mouseSensitivityX * deltaTime;

    const float pitchDirection = settings.invertPitchInput ? -1.0f : 1.0f;
    state.targetPitch += mouseY * settings.mouseSensitivityY * pitchDirection * deltaTime;
}

// カメラの前方と右方向のベクトルを構築
void CameraControlBehavior::BuildCameraBasis(XMFLOAT3& outForward, XMFLOAT3& outRight) const
{
    const CameraRuntimeState& state = m_context.runtimeState;
    XMFLOAT4 cameraQuaternion = MiMath::QuaternionFromEuler({ state.pitch, state.yaw, 0.0f });

    outForward = MiMath::RotateVector(cameraQuaternion, { 0.0f, 0.0f, 1.0f });
    outRight = MiMath::RotateVector(cameraQuaternion, { 1.0f, 0.0f, 0.0f });
}

/// @brief 追従アンカー位置のターゲット値を計算する
XMFLOAT3 CameraControlBehavior::CalculateTargetFollowAnchorPosition()
{
    XMFLOAT3 targetPosition = m_context.references.targetTransform->GetPosition();
    return targetPosition;
}

/// @brief 追従アンカー位置を滑らかに更新する
void CameraControlBehavior::UpdateFollowAnchorPosition(const XMFLOAT3 targetPosition, float deltaTime)
{
    CameraRuntimeState& state = m_context.runtimeState;
    const CameraSettings::Data& settings = m_context.settings();

    // === 平面方向の追従 ===
    state.followAnchorPosition.x = MiMath::SmoothDamp(
        state.followAnchorPosition.x,
        targetPosition.x,
        state.followAnchorVelocity.x,
        settings.followAnchorPlanarSmoothTime,
        deltaTime);
    state.followAnchorPosition.z = MiMath::SmoothDamp(
        state.followAnchorPosition.z,
        targetPosition.z,
        state.followAnchorVelocity.z,
        settings.followAnchorPlanarSmoothTime,
        deltaTime);

    // === 上下方向の追従 ===
    float smoothTime = 0.0f;
    if (state.followAnchorPosition.y > targetPosition.y) {
        // 追従アンカーがターゲットより上にある場合は、下方向の追従時間を使用
        smoothTime = settings.followAnchorDownwardSmoothTime;
    }
    else {
        // 追従アンカーがターゲットより下にある場合は、上方向の追従時間を使用
        smoothTime = settings.followAnchorUpwardSmoothTime;
    }

    state.followAnchorPosition.y = MiMath::SmoothDamp(
        state.followAnchorPosition.y,
        targetPosition.y,
        state.followAnchorVelocity.y,
        smoothTime,
        deltaTime);
}

/// @brief カメラの注視点のオフセットを計算する
XMFLOAT3 CameraControlBehavior::CalculateCompositionOffset()
{
    const CameraRuntimeState& state = m_context.runtimeState;

    const XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 localOffset = { 0.0f, 0.0f, 0.0f };
    localOffset = MiMath::Add(localOffset, MiMath::Multiply(state.cameraRight, state.compositionCameraLocalOffset.x));
    localOffset = MiMath::Add(localOffset, MiMath::Multiply(worldUp, state.compositionCameraLocalOffset.y));
    localOffset = MiMath::Add(localOffset, MiMath::Multiply(state.cameraForward, state.compositionCameraLocalOffset.z));

    XMFLOAT3 totalOffset = MiMath::Add(state.compositionWorldOffset, localOffset);
    totalOffset.y += m_context.settings().lookAtHeight;

    return totalOffset;
}

float CameraControlBehavior::EvaluateCameraDistance() const
{
    float distance = m_context.runtimeState.followDistance;
    if (m_context.runtimeState.pitch <= m_context.settings().minPitchTreshold && 
        m_context.runtimeState.pitch >= m_context.settings().minPitch) 
    {
        float t = (m_context.settings().minPitchTreshold - m_context.runtimeState.pitch) /
            (m_context.settings().minPitchTreshold - m_context.settings().minPitch);
        t = MiMath::Clamp(t, 0.0f, 1.0f);
        distance *= MiMath::Lerp(1.0f, m_context.settings().minPitchDistanceMultiplier, t);
    }
    if (m_context.runtimeState.pitch >= m_context.settings().maxPitchTreshold && 
        m_context.runtimeState.pitch <= m_context.settings().maxPitch) 
    {
        float t = (m_context.runtimeState.pitch - m_context.settings().maxPitchTreshold) /
            (m_context.settings().maxPitch - m_context.settings().maxPitchTreshold);
        t = MiMath::Clamp(t, 0.0f, 1.0f);
        distance *= MiMath::Lerp(1.0f, m_context.settings().maxPitchDistanceMultiplier, t);
    }
    return distance;
}
