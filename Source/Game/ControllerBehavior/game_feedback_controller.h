//---------------------------------------------------
// game_effect_controller.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/27
//---------------------------------------------------
#ifndef GAME_EFFECT_CONTROLLER_H
#define GAME_EFFECT_CONTROLLER_H
#include "Engine/Component/behavior_component.h"
#include "Engine/Device/direct3d.h"
using namespace DirectX;
#include "Engine/Core/GamePlay/sequence_task.h"
#include "Game/PresBehavior/UI/ui_handle.h"

#include <cstdint>
#include <string>
#include <vector>

class CameraControlBehavior;

class GameFeedbackController : public BehaviorComponent {
public:
    using FlashId = std::uint64_t;
    static constexpr FlashId InvalidFlashId = 0;

private:
    // カメラコントロールビヘイビアへの参照
    CameraControlBehavior* m_cameraControl = nullptr;

public:
    GameFeedbackController() = default;
    ~GameFeedbackController();

    void Start() override;
    void Update() override;
    void OnDestroy() override;
    void DrawComponentInspector() override;

private:
    // タイムスケール変更タスク
    class ChangeTimeScaleTask : public SequenceTask {
    public:
        static constexpr float m_defaultTimeScale = 1.0f; // デフォルトのタイムスケール

        // タイムスケールの開始値と目標値
        float m_startTimeScale = 1.0f;
        float m_targetTimeScale = 1.0f;
        // 時間経過の管理
        float m_duration = 0.0f;
        float m_holdDuration = 0.0f;

        void Update(float deltaTime) override;
    };
    ChangeTimeScaleTask m_changeTimeScaleTask;

    enum class FlashState {
        FadeIn,
        Hold,
        Keep,
        FadeOut,
    };

    struct FlashLayer {
        FlashId id = InvalidFlashId;
        UiHandle image;
        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        float startAlpha = 0.0f;
        float duration = 0.0f;
        float holdDuration = 0.0f;
        float timer = 0.0f;
        FlashState state = FlashState::FadeIn;
        bool temporary = false;
    };

    std::vector<FlashLayer> m_flashLayers;
    FlashId m_nextFlashId = 1;

    FlashId CreateFlash(const std::wstring& texturePath, const XMFLOAT4& color,
        float duration, float holdDuration, bool temporary);
    void UpdateFlashLayers(float deltaTime);
    void DestroyFlashLayersImmediate();

public:
    // タイムスケール変更
    void ChangeTimeScale(float timeScale, float duration);
    void ChangeTimeScaleTemporary(float timeScale, float duration, float holdDuration);
    // タイムスケールを元に戻す
    void ResetTimeScale(float duration);

    // FOV変更
    void ChangeFOV(float fov, float duration);
    void ChangeFOVTemporary(float fov, float duration, float holdDuration);
    // FOVを元に戻す
    void ResetFOV(float duration);

    // カメラシェイク再生
    void PlayCameraShake(float duration, float magnitude);

    // 一時フラッシュ。フェードイン後に保持し、同じdurationでフェードアウトする。
    FlashId PlayFlash(const std::wstring& texturePath, const XMFLOAT4& color,
        float duration, float holdDuration);
    // 指定画像をフェードインし、解除されるまで保持する。
    FlashId ChangeFlash(const std::wstring& texturePath, const XMFLOAT4& color,
        float duration);
    // 指定したフラッシュ、または全フラッシュをフェードアウトして破棄する。
    void ResetFlash(FlashId id, float duration);
    void ResetAllFlashes(float duration);

};

#endif // GAME_EFFECT_CONTROLLER_H
