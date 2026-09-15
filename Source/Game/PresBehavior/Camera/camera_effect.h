//---------------------------------------------------
// File  ：_/PresBehavior/Camera/camera_effect.h
// Date  ：2026/08/15
// Author：Miu Kitamura
// 
// ・CameraControlBehaviorのカメラエフェクト関連の処理をまとめたクラス
//---------------------------------------------------
#pragma once
#include "Engine/Core/GamePlay/sequence_task.h"
#include "Engine/Core/GamePlay/tween_task.h"

class CameraContext;

class CameraEffect {
public:
    /// @brief カメラエフェクトのターゲットを表す列挙型
    enum class EffectTaskTarget {
        FOV,
        Distance,
        CompositionWorldOffset,
        CompositionCameraLocalOffset
    };

    /// @brief カメラエフェクトのタスクの種類を表す列挙型
    enum class EffectTaskType {
        Change,
        ChangeTemporary,
        Reset
    };

    /// @brief カメラエフェクトのリクエスト情報を表す構造体
    struct RequestEffectTaskInfo 
    {
        EffectTaskType taskType;
        XMFLOAT3 targetValue; // Change, ChangeTemporaryの場合に使用

        float duration;
        float holdDuration;   // ChangeTemporaryの場合に使用
    };

private:
    // === カメラシェイク設定 ===
    bool m_isShaking = false;
    XMFLOAT3 m_shakeOffset = { 0.0f, 0.0f, 0.0f }; // シェイクによる位置のオフセット

    // FOV変更タスク
    FloatTweenTask m_fovTask;
    // カメラ距離変更タスク
    FloatTweenTask m_cameraDistanceTask;
    // カメラオフセット変更タスク
    Vector3TweenTask m_compositionWorldOffsetTask;
    Vector3TweenTask m_compositionCameraLocalOffsetTask;

    // カメラシェイクタスク
    class CameraShakeTask : public SequenceTask {
    public:
        float m_duration = 0.0f;
        float m_magnitude = 0.0f;
        float m_shakeFrequency = 35.0f;
        XMFLOAT3 m_shakeOffset = { 0.0f, 0.0f, 0.0f };
        void Update(float deltaTime) override;
    };
    CameraShakeTask m_cameraShakeTask;

public:
    /// @brief カメラエフェクトを初期化する
    void Initialize(CameraContext& context);

    /// @brief カメラエフェクトタスクを更新する
    void UpdateCameraEffectTasks(CameraContext& context, float deltaTime);

    /// @brief カメラエフェクトタスクをリクエストする
    void RequestEffectTask(CameraContext& context, EffectTaskTarget target, const RequestEffectTaskInfo& requestInfo);
    /// @brief カメラシェイクを再生する
    void PlayCameraShake(CameraContext& context, float duration, float magnitude);

    bool IsCameraShaking() const { return m_isShaking; }
    const XMFLOAT3& GetShakeOffset() const { return m_shakeOffset; }

private:

};
