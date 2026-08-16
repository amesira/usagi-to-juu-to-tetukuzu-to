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

public:
    ~CameraControlBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // ユーザーによるカメラ回転入力の有効状態
    void SetCameraInputEnabled(bool enabled);
    bool IsCameraInputEnabled() const;

    /// @brief カメラエフェクトタスクをリクエストする
    void RequestCameraEffectTask(CameraEffect::EffectTaskTarget target, const CameraEffect::RequestEffectTaskInfo& requestInfo);

    // カメラシェイク再生
    void PlayCameraShake(float duration, float magnitude);

private:
    // 入力の有効・無効を切り替える操作を処理
    void UpdateCameraInputActivation();

    // カメラの基底ベクトルの構築
    void BuildCameraBasis(XMFLOAT3& outForward, XMFLOAT3& outRight) const;

    // カメラの注視点のターゲット値を計算
    XMFLOAT3 CalculateTargetAtPosition();
    // カメラ回転のターゲット値の入力による更新
    void UpdateTargetYawPitchFromInput(float deltaTime);

    // カメラ位置を計算
    XMFLOAT3 CalculateTargetCameraPosition(const XMFLOAT3& cameraForward, const XMFLOAT3& cameraRight, const XMFLOAT3& targetAtPosition);
};


namespace CameraEffectTaskHelper {

    inline void ChangeCameraEffect(CameraControlBehavior* cameraControl, CameraEffect::EffectTaskTarget effectTaskTarget, float target, float duration)
    {
        if (!cameraControl) return;
        cameraControl->RequestCameraEffectTask(effectTaskTarget, {
            .taskType = CameraEffect::EffectTaskType::Change,
            .targetValue = { target, 0.0f, 0.0f },
            .duration = duration
            });
    }
    inline void ChangeCameraEffect(CameraControlBehavior* cameraControl, CameraEffect::EffectTaskTarget effectTaskTarget, const XMFLOAT3& target, float duration)
    {
        if (!cameraControl) return;
        cameraControl->RequestCameraEffectTask(effectTaskTarget, {
            .taskType = CameraEffect::EffectTaskType::Change,
            .targetValue = target,
            .duration = duration
            });
    }

    inline void ChangeCameraEffectTemporary(CameraControlBehavior* cameraControl, CameraEffect::EffectTaskTarget effectTaskTarget, float target, float duration, float holdDuration)
    {
        if (!cameraControl) return;
        cameraControl->RequestCameraEffectTask(effectTaskTarget, {
            .taskType = CameraEffect::EffectTaskType::ChangeTemporary,
            .targetValue = { target, 0.0f, 0.0f },
            .duration = duration,
            .holdDuration = holdDuration
            });
    }
    inline void ChangeCameraEffectTemporary(CameraControlBehavior* cameraControl, CameraEffect::EffectTaskTarget effectTaskTarget, const XMFLOAT3& target, float duration, float holdDuration)
    {
        if (!cameraControl) return;
        cameraControl->RequestCameraEffectTask(effectTaskTarget, {
            .taskType = CameraEffect::EffectTaskType::ChangeTemporary,
            .targetValue = target,
            .duration = duration,
            .holdDuration = holdDuration
            });
    }

    inline void ResetCameraEffect(CameraControlBehavior* cameraControl, CameraEffect::EffectTaskTarget effectTaskTarget, float duration)
    {
        if (!cameraControl) return;
        cameraControl->RequestCameraEffectTask(effectTaskTarget, {
            .taskType = CameraEffect::EffectTaskType::Reset,
            .duration = duration
            });
    }
}

#endif // CAMERA_CONTROL_BEHAVIOR_H
