//===================================================
// File  ：_/PresBehavior/Camera/camera_effect.cpp
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・CameraControlBehaviorのカメラエフェクト関連の処理をまとめたクラス
//===================================================
#include "camera_effect.h"

#include "Utility/mi_math.h"
#include "Utility/mi_signal.h"

#include "camera_context.h"
#include "camera_control_behavior.h"

#include "Engine/Framework/Component/camera_component.h"

/// @brief カメラエフェクトを初期化する
void CameraEffect::Initialize(CameraContext& context)
{
    m_fovTask.Reset();
    m_cameraDistanceTask.Reset();
    m_cameraOffsetTask.Reset();
    m_cameraLocalOffsetTask.Reset();
}

/// @brief カメラエフェクトタスクを更新する
void CameraEffect::UpdateCameraEffectTasks(CameraContext& context, float deltaTime)
{
    //タスクが実行中かどうか
    bool fovTaskRunning = !m_fovTask.IsFinished();
    bool distanceTaskRunning = !m_cameraDistanceTask.IsFinished();
    bool offsetTaskRunning = !m_cameraOffsetTask.IsFinished();
    bool localOffsetTaskRunning = !m_cameraLocalOffsetTask.IsFinished();
    bool cameraShakeTaskRunning = !m_cameraShakeTask.IsFinished();

    // タスクの更新
    m_fovTask.Update(deltaTime);
    m_cameraDistanceTask.Update(deltaTime);
    m_cameraOffsetTask.Update(deltaTime);
    m_cameraLocalOffsetTask.Update(deltaTime);
    m_cameraShakeTask.Update(deltaTime);

    // タスクの更新後に値を適用
    if (fovTaskRunning && context.camera) {
        context.runtimeState.fov = m_fovTask.m_currentValue;
        context.camera->SetFov(context.runtimeState.fov);
    }
    if (distanceTaskRunning) {
        context.runtimeState.followDistance = m_cameraDistanceTask.m_currentValue;
    }
    if (offsetTaskRunning) {
        context.runtimeState.lookAtOffset = m_cameraOffsetTask.m_currentValue;
    }
    if (localOffsetTaskRunning) {
        context.runtimeState.lookAtLocalOffset = m_cameraLocalOffsetTask.m_currentValue;
    }
    if (cameraShakeTaskRunning) {
        m_isShaking = true;
        m_shakeOffset = m_cameraShakeTask.m_shakeOffset;
    }
    else {
        m_isShaking = false;
    }
}

/// @brief カメラエフェクトタスクをリクエストする
void CameraEffect::RequestEffectTask(CameraContext& context, EffectTaskTarget target, const RequestEffectTaskInfo& requestInfo)
{
    TweenTask* task = nullptr;
    EffectTaskType taskType = requestInfo.taskType;
    
    // ターゲットに応じてタスクを選択し、開始値、目標値、終了値を設定する
    switch (target) {
        case EffectTaskTarget::FOV: {
            task = &m_fovTask;
            m_fovTask.m_startValue = context.runtimeState.fov;
            m_fovTask.m_targetValue = (taskType == EffectTaskType::Reset)
                ? context.settings().fov
                : requestInfo.targetValue.x;
            m_fovTask.m_endValue = (taskType == EffectTaskType::ChangeTemporary)
                ? context.settings().fov
                : m_fovTask.m_targetValue;
            break;
        }
        case EffectTaskTarget::Distance: {
            task = &m_cameraDistanceTask;
            m_cameraDistanceTask.m_startValue = context.runtimeState.followDistance;
            m_cameraDistanceTask.m_targetValue = (taskType == EffectTaskType::Reset)
                ? context.settings().followDistance
                : requestInfo.targetValue.x;
            m_cameraDistanceTask.m_endValue = (taskType == EffectTaskType::ChangeTemporary)
                ? context.settings().followDistance
                : m_cameraDistanceTask.m_targetValue;
            break;
        }
        case EffectTaskTarget::Offset: {
            task = &m_cameraOffsetTask;
            m_cameraOffsetTask.m_startValue = context.runtimeState.lookAtOffset;
            m_cameraOffsetTask.m_targetValue = (taskType == EffectTaskType::Reset)
                ? context.settings().lookAtOffset
                : requestInfo.targetValue;
            m_cameraOffsetTask.m_endValue = (taskType == EffectTaskType::ChangeTemporary)
                ? context.settings().lookAtOffset
                : m_cameraOffsetTask.m_targetValue;
            break;
        }
        case EffectTaskTarget::LocalOffset: {
            task = &m_cameraLocalOffsetTask;
            m_cameraLocalOffsetTask.m_startValue = context.runtimeState.lookAtLocalOffset;
            m_cameraLocalOffsetTask.m_targetValue = (taskType == EffectTaskType::Reset)
                ? context.settings().lookAtLocalOffset
                : requestInfo.targetValue;
            m_cameraLocalOffsetTask.m_endValue = (taskType == EffectTaskType::ChangeTemporary)
                ? context.settings().lookAtLocalOffset
                : m_cameraLocalOffsetTask.m_targetValue;
            break;
        }
        default: break;
    }

    if (task == nullptr) return;

    // タスクの種類に応じて、補間時間や保持時間を設定する
    switch(taskType) {
        case EffectTaskType::Change: {
            task->m_duration = requestInfo.duration;
            task->m_holdDuration = 0.0f;
            break;
        }
        case EffectTaskType::ChangeTemporary: {
            task->m_duration = requestInfo.duration;
            task->m_holdDuration = requestInfo.holdDuration;
            break;
        }
        case EffectTaskType::Reset: {
            task->m_duration = requestInfo.duration;
            task->m_holdDuration = 0.0f;
            break;
        }
        default: break;
    }

    // タスクを開始する
    if (task) {
        task->Start();
    }
}

/// @brief カメラシェイクを再生する
void CameraEffect::PlayCameraShake(CameraContext& context, float duration, float magnitude)
{
    m_cameraShakeTask.Reset();
    m_cameraShakeTask.m_duration = duration;
    m_cameraShakeTask.m_magnitude = magnitude;
    m_cameraShakeTask.m_shakeFrequency = context.settingsAsset
        ? context.settings().shakeFrequency
        : 35.0f;
    m_cameraShakeTask.Start();
}

// ----- CameraShakeTaskの実装 -----

/// @brief カメラシェイクタスクの更新処理
void CameraEffect::CameraShakeTask::Update(float deltaTime)
{
    SequenceTask::Update(deltaTime);
    if (IsFinished()) return;

    switch (m_taskStep) {
    case 0: {
        float t = 1.0f;
        if (m_duration > 0.0f) {
            t = (std::min)(m_taskTimer / m_duration, 1.0f);
        }

        float elapsed = m_taskTimer;

        // シェイクの強さを時間経過に応じて減衰させる
        m_magnitude = MiMath::Lerp(m_magnitude, 0.0f, t * t);

        // Perlinノイズを使用してシェイクのオフセットを生成
        m_shakeOffset = { 0.0f, 0.0f, 0.0f };
        m_shakeOffset.x = MiSignal::Perlin1D(elapsed * m_shakeFrequency) * m_magnitude;
        m_shakeOffset.y = MiSignal::Perlin1D((elapsed + 100.0f) * m_shakeFrequency) * m_magnitude;

        // タスクの終了判定
        if (m_taskTimer >= m_duration) {
            Finish();
        }
        break;
    }
    }
}
