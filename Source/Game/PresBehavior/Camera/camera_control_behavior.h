//---------------------------------------------------
// camera_control_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/08
//---------------------------------------------------
#ifndef CAMERA_CONTROL_BEHAVIOR_H
#define CAMERA_CONTROL_BEHAVIOR_H
#include "Engine/Framework/Component/behavior_component.h"
#include "Engine/Core/GamePlay/sequence_task.h"
#include "Engine/Core/GamePlay/tween_task.h"
#include "camera_context.h"

#include <DirectXMath.h>
using namespace DirectX;

class TransformComponent;
class CameraComponent;
class RigidbodyComponent;

// カメラ制御用のBehaviorComponent
class CameraControlBehavior : public BehaviorComponent {
private:
    CameraContext m_context;

    std::uint64_t m_lastSettingsRevision = 0; // DataAssetのリビジョン番号を保持し、変更があった場合に更新する
    std::function<void()> m_settingsReloadCallback; // DataAssetのリロード時に呼ばれるコールバック

    // === カメラシェイク設定 ===
    bool m_isShaking = false;
    XMFLOAT3 m_shakeOffset = { 0.0f, 0.0f, 0.0f }; // シェイクによる位置のオフセット

    // FOV変更タスク
    FloatTweenTask m_fovTask;
    // カメラ距離変更タスク
    FloatTweenTask m_cameraDistanceTask;
    // カメラオフセット変更タスク
    Vector3TweenTask m_cameraOffsetTask;
    Vector3TweenTask m_cameraLocalOffsetTask;

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
    ~CameraControlBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // ユーザーによるカメラ回転入力の有効状態
    void SetCameraInputEnabled(bool enabled);
    bool IsCameraInputEnabled() const;

    // FOV変更
    void ChangeFOV(float fov, float duration);
    void ChangeFOVTemporary(float fov, float duration, float holdDuration);
    // FOVを元に戻す
    void ResetFOV(float duration);

    // カメラ距離変更
    void ChangeCameraDistance(float distance, float duration);
    void ChangeCameraDistanceTemporary(float distance, float duration, float holdDuration);
    // カメラ距離を元に戻す
    void ResetCameraDistance(float duration);

    // カメラオフセット変更
    void ChangeCameraOffset(const XMFLOAT3& offset, float duration);
    void ChangeCameraOffsetTemporary(const XMFLOAT3& offset, float duration, float holdDuration);
    // カメラオフセットを元に戻す
    void ResetCameraOffset(float duration);

    void ChangeCameraLocalOffset(const XMFLOAT3& offset, float duration);
    void ChangeCameraLocalOffsetTemporary(const XMFLOAT3& offset, float duration, float holdDuration);
    void ResetCameraLocalOffset(float duration);

    // カメラシェイク再生
    void PlayCameraShake(float duration, float magnitude);

private:
    // 入力の有効・無効を切り替える操作を処理
    void UpdateCameraInputActivation();

    // エフェクトタスクの更新
    void UpdateCameraEffectTasks(float deltaTime);

    // カメラの基底ベクトルの構築
    void BuildCameraBasis(XMFLOAT3& outForward, XMFLOAT3& outRight) const;

    // カメラの注視点のターゲット値を計算
    XMFLOAT3 CalculateTargetAtPosition();
    // カメラ回転のターゲット値の入力による更新
    void UpdateTargetYawPitchFromInput(float deltaTime);

    // カメラ位置を計算
    XMFLOAT3 CalculateTargetCameraPosition(const XMFLOAT3& cameraForward, const XMFLOAT3& cameraRight, const XMFLOAT3& targetAtPosition);
};

#endif // CAMERA_CONTROL_BEHAVIOR_H
