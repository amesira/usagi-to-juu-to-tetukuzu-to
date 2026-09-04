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

GameFeedbackController::GameFeedbackController()
{
    s_instanceCount++;

    if (s_instanceCount > 1) {
        hal::dout << "警告: GameEffectControllerのインスタンスが複数存在しています。ゲーム全体で1つだけ存在することを想定しています。" << std::endl;
        this->SetEnable(false);
    }
    else {
        GameControllerLocator::s_gameEffectController = this;
    }

    // タスクの初期化
    m_changeTimeScaleTask.Reset();
}

GameFeedbackController::~GameFeedbackController()
{
    if (GameControllerLocator::s_gameEffectController == this) {
        GameControllerLocator::s_gameEffectController = nullptr;
    }

    s_instanceCount--;
}

void GameFeedbackController::Start()
{
    IScene* scene = GetOwner()->GetScene();

    // カメラコントロールビヘイビアへの参照取得
    {
        GameObject* mainCamera = scene->GetGameObjectByName("MainCamera");
        if (mainCamera) {
            m_cameraControl = mainCamera->GetComponent<CameraControlBehavior>();
        }
    }
}

void GameFeedbackController::Update()
{
    float unscaledDeltaTime = FPS_GetUnscaledDeltaTime();

    // タスクの更新
    m_changeTimeScaleTask.Update(unscaledDeltaTime);
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
