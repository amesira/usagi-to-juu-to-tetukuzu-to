//===================================================
// game_feedback_controller.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/04/27
//===================================================
#include "game_feedback_controller.h"

#include "Engine/Device/mi_fps.h"
#include "game_controller_locator.h"

#include "Engine/Editor/LevelEditor/behavior_detail_view.h"

#include "Utility/mi_math.h"

#include "Utility/debug_ostream.h"

#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include "Game/PresBehavior/Camera/camera_control_behavior.h"
#include "Game/Factory/ui_factory.h"

#include "Engine/Component/image_component.h"
#include "Engine/Component/rect_transform_component.h"

#include <algorithm>

namespace {
    constexpr float FLASH_BASE_ORDER = 10000.0f;
}

GameFeedbackController::~GameFeedbackController()
{
    if (GameControllerLocator::s_gameEffectController == this) {
        GameControllerLocator::s_gameEffectController = nullptr;
    }
}

void GameFeedbackController::Start()
{
    auto* current = GameControllerLocator::s_gameEffectController;
    if (current && current != this && current->GetEnable()) {
        SetEnable(false);
        return;
    }
    GameControllerLocator::s_gameEffectController = this;

    IScene* scene = GetOwner()->GetScene();

    // カメラコントロールビヘイビアへの参照取得
    GameObject* mainCamera = scene->GetGameObjectByName("MainCamera");
    if (mainCamera) {
        m_cameraControl = mainCamera->GetComponent<CameraControlBehavior>();
    }

    // タスクの初期化
    m_changeTimeScaleTask.Reset();
}

void GameFeedbackController::Update()
{
    float unscaledDeltaTime = FPS_GetUnscaledDeltaTime();

    // タスクの更新
    m_changeTimeScaleTask.Update(unscaledDeltaTime);
    UpdateFlashLayers(unscaledDeltaTime);
}

void GameFeedbackController::OnDestroy()
{
    DestroyFlashLayersImmediate();

    if (GameControllerLocator::s_gameEffectController == this) {
        GameControllerLocator::s_gameEffectController = nullptr;
    }
}

void GameFeedbackController::DrawComponentInspector()
{
    if (BehaviorDetailView::BeginSection(this, "Game Effect Controller")) {
        if (ImGui::TreeNode("Time Scale")) {
            static float timeScale = 0.2f;
            static float duration = 0.15f;
            static float holdDuration = 0.2f;

            ImGui::DragFloat("Target Time Scale", &timeScale, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat("Duration", &duration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Hold Duration", &holdDuration, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Change Time Scale")) {
                ChangeTimeScale(timeScale, duration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Temporary")) {
                ChangeTimeScaleTemporary(timeScale, duration, holdDuration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                ResetTimeScale(duration);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Camera FOV")) {
            static float fov = 60.0f;
            static float duration = 0.2f;
            static float holdDuration = 0.2f;

            ImGui::DragFloat("FOV", &fov, 1.0f, 1.0f, 179.0f);
            ImGui::DragFloat("Duration", &duration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Hold Duration", &holdDuration, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Change FOV")) {
                ChangeFOV(fov, duration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Temporary")) {
                ChangeFOVTemporary(fov, duration, holdDuration);
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset")) {
                ResetFOV(duration);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Camera Shake")) {
            static float duration = 0.2f;
            static float magnitude = 0.2f;

            ImGui::DragFloat("Duration", &duration, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("Magnitude", &magnitude, 0.01f, 0.0f, 5.0f);

            if (ImGui::Button("Play Camera Shake")) {
                PlayCameraShake(duration, magnitude);
            }

            ImGui::TreePop();
        }
    }

    BehaviorDetailView::EndSection();
}

//------------------------------- private

#pragma region Flash
GameFeedbackController::FlashId GameFeedbackController::CreateFlash(
    const std::wstring& texturePath, const XMFLOAT4& color,
    float duration, float holdDuration, bool temporary)
{
    IScene* scene = GetOwner() ? GetOwner()->GetScene() : nullptr;
    if (!scene || texturePath.empty()) return InvalidFlashId;

    UiHandle image = UiFactory::CreateUiImageHandle(scene, texturePath);
    if (!image.IsValid()) return InvalidFlashId;

    FlashId id = m_nextFlashId++;
    if (id == InvalidFlashId) id = m_nextFlashId++;

    const float width = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float height = static_cast<float>(Direct3D_GetBackBufferHeight());
    image.SetPosition(width * 0.5f, height * 0.5f);
    image.SetSize(width, height);
    image.SetColor({ color.x, color.y, color.z });
    image.SetAlpha(0.0f);

    if (RectTransformComponent* rect = image.GetRectTransform()) {
        XMFLOAT3 position = rect->GetPosition();
        position.z = FLASH_BASE_ORDER + static_cast<float>(id);
        rect->SetPosition(position);
    }

    FlashLayer layer;
    layer.id = id;
    layer.image = image;
    layer.color = {
        color.x,
        color.y,
        color.z,
        std::clamp(color.w, 0.0f, 1.0f)
    };
    layer.duration = (std::max)(duration, 0.0f);
    layer.holdDuration = (std::max)(holdDuration, 0.0f);
    layer.temporary = temporary;
    m_flashLayers.push_back(layer);

    return id;
}

void GameFeedbackController::UpdateFlashLayers(float deltaTime)
{
    const float width = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float height = static_cast<float>(Direct3D_GetBackBufferHeight());

    for (std::size_t i = 0; i < m_flashLayers.size();) {
        FlashLayer& layer = m_flashLayers[i];
        if (!layer.image.IsValid()) {
            m_flashLayers.erase(m_flashLayers.begin() + i);
            continue;
        }

        layer.image.SetPosition(width * 0.5f, height * 0.5f);
        layer.image.SetSize(width, height);
        layer.timer += (std::max)(deltaTime, 0.0f);

        switch (layer.state) {
        case FlashState::FadeIn: {
            const float t = layer.duration > 0.0f
                ? std::clamp(layer.timer / layer.duration, 0.0f, 1.0f)
                : 1.0f;
            layer.image.SetAlpha(MiMath::Lerp(0.0f, layer.color.w, t));
            if (t >= 1.0f) {
                layer.timer = 0.0f;
                layer.state = layer.temporary ? FlashState::Hold : FlashState::Keep;
            }
            break;
        }
        case FlashState::Hold:
            layer.image.SetAlpha(layer.color.w);
            if (layer.timer >= layer.holdDuration) {
                layer.timer = 0.0f;
                layer.startAlpha = layer.color.w;
                layer.state = FlashState::FadeOut;
            }
            break;

        case FlashState::Keep:
            layer.image.SetAlpha(layer.color.w);
            break;

        case FlashState::FadeOut: {
            const float t = layer.duration > 0.0f
                ? std::clamp(layer.timer / layer.duration, 0.0f, 1.0f)
                : 1.0f;
            layer.image.SetAlpha(MiMath::Lerp(layer.startAlpha, 0.0f, t));
            if (t >= 1.0f) {
                layer.image.Destroy();
                m_flashLayers.erase(m_flashLayers.begin() + i);
                continue;
            }
            break;
        }
        }

        ++i;
    }
}

void GameFeedbackController::DestroyFlashLayersImmediate()
{
    for (FlashLayer& layer : m_flashLayers) {
        layer.image.Destroy();
    }
    m_flashLayers.clear();
}
#pragma endregion

// タイムスケール変更タスクの更新
void GameFeedbackController::ChangeTimeScaleTask::Update(float deltaTime)
{
    if (!m_isRunning) return;
    SequenceTask::Update(deltaTime);

    switch(m_taskStep) {
        case 0: {
            // タイムスケールを目標値に向けて変化させる
            float t = 1.0f;
            if (m_duration > 0.0f) {
                t = (std::min)(m_taskTimer / m_duration, 1.0f);
            }
            float newTimeScale = MiMath::Lerp(m_startTimeScale, m_targetTimeScale, t);
            FPS_SetTimeScale(newTimeScale);

            if (t >= 1.0f) {
                AdvanceStep();
            }
            break;
        }
        case 1: {
            if (m_holdDuration > 0.0f) {
                // 一定時間保持
                if (Wait(m_holdDuration)) {
                    AdvanceStep();
                }
            }
            else {
                Finish(); // 保持なしで完了
            }
            break;
        }
        case 2: {
            // タイムスケールを元に戻す
            float t = 1.0f;
            if (m_duration > 0.0f) {
                t = (std::min)(m_taskTimer / m_duration, 1.0f);
            }
            float newTimeScale = MiMath::Lerp(m_targetTimeScale, m_defaultTimeScale, t);
            FPS_SetTimeScale(newTimeScale);

            if (t >= 1.0f) {
                FPS_SetTimeScale(m_defaultTimeScale); // 確実に元のタイムスケールに戻す
                Finish();
            }
            break;
        }
    }
}


//------------------------------- public

// タイムスケール変更
void GameFeedbackController::ChangeTimeScale(float timeScale, float duration)
{
    m_changeTimeScaleTask.Reset();

    m_changeTimeScaleTask.m_startTimeScale = FPS_GetTimeScale();
    m_changeTimeScaleTask.m_targetTimeScale = timeScale;
    m_changeTimeScaleTask.m_duration = duration;
    m_changeTimeScaleTask.m_holdDuration = 0.0f;
    m_changeTimeScaleTask.Start();
}
void GameFeedbackController::ChangeTimeScaleTemporary(float timeScale, float duration, float holdDuration)
{
    m_changeTimeScaleTask.Reset();

    m_changeTimeScaleTask.m_startTimeScale = FPS_GetTimeScale();
    m_changeTimeScaleTask.m_targetTimeScale = timeScale;
    m_changeTimeScaleTask.m_duration = duration;
    m_changeTimeScaleTask.m_holdDuration = holdDuration;
    m_changeTimeScaleTask.Start();
}
// タイムスケールを元に戻す
void GameFeedbackController::ResetTimeScale(float duration)
{
    ChangeTimeScale(1.0f, duration);
}

// ----- CameraControlBehaviorを介した実装の窓口

// FOV変更
void GameFeedbackController::ChangeFOV(float fov, float duration)
{
    if (m_cameraControl) {
        CameraEffectTaskHelper::ChangeCameraEffect(m_cameraControl, CameraEffect::EffectTaskTarget::FOV, fov, duration);
    }
}
void GameFeedbackController::ChangeFOVTemporary(float fov, float duration, float holdDuration)
{
    if (m_cameraControl) {
        CameraEffectTaskHelper::ChangeCameraEffectTemporary(m_cameraControl, CameraEffect::EffectTaskTarget::FOV, fov, duration, holdDuration);
    }
}
// FOVを元に戻す
void GameFeedbackController::ResetFOV(float duration)
{
    if (m_cameraControl) {
        CameraEffectTaskHelper::ResetCameraEffect(m_cameraControl, CameraEffect::EffectTaskTarget::FOV, duration);
    }
}

// カメラシェイク再生
void GameFeedbackController::PlayCameraShake(float duration, float magnitude)
{
    if (m_cameraControl) {
        m_cameraControl->PlayCameraShake(duration, magnitude);
    }
}

GameFeedbackController::FlashId GameFeedbackController::PlayFlash(
    const std::wstring& texturePath, const XMFLOAT4& color,
    float duration, float holdDuration)
{
    return CreateFlash(texturePath, color, duration, holdDuration, true);
}

GameFeedbackController::FlashId GameFeedbackController::ChangeFlash(
    const std::wstring& texturePath, const XMFLOAT4& color, float duration)
{
    return CreateFlash(texturePath, color, duration, 0.0f, false);
}

void GameFeedbackController::ResetFlash(FlashId id, float duration)
{
    if (id == InvalidFlashId) return;

    auto it = std::find_if(m_flashLayers.begin(), m_flashLayers.end(),
        [id](const FlashLayer& layer) { return layer.id == id; });
    if (it == m_flashLayers.end()) return;

    if (duration <= 0.0f) {
        it->image.Destroy();
        m_flashLayers.erase(it);
        return;
    }

    ImageComponent* image = it->image.GetImage();
    it->startAlpha = image ? image->GetColor().w : 0.0f;
    it->duration = duration;
    it->holdDuration = 0.0f;
    it->timer = 0.0f;
    it->state = FlashState::FadeOut;
}

void GameFeedbackController::ResetAllFlashes(float duration)
{
    if (duration <= 0.0f) {
        DestroyFlashLayersImmediate();
        return;
    }

    for (FlashLayer& layer : m_flashLayers) {
        ImageComponent* image = layer.image.GetImage();
        layer.startAlpha = image ? image->GetColor().w : 0.0f;
        layer.duration = duration;
        layer.holdDuration = 0.0f;
        layer.timer = 0.0f;
        layer.state = FlashState::FadeOut;
    }
}
