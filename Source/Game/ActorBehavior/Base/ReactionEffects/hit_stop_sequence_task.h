#pragma once
#include <functional>
#include "Engine/Core/GamePlay/sequence_task.h"

// ヒットストップ処理を管理するタスククラス
class HitStopTask : public SequenceTask {
private:
    using HitStopCallback = std::function<void()>;

public:
    float m_duration = 0.0f;

    HitStopCallback m_onEntered = nullptr;
    HitStopCallback m_onUpdate = nullptr;
    HitStopCallback m_onExit = nullptr;

    void Start() override {
        SequenceTask::Start();
    }
    void Update(float deltaTime) override {
        if (IsFinished()) return;
        SequenceTask::Update(deltaTime);

        switch (m_taskStep) {
        case 0:
            AdvanceStep();
            break;
        case 1:
            if (m_onEntered) {
                m_onEntered();
            }
            AdvanceStep();
            break;
        case 2:
            if (m_onUpdate) {
                m_onUpdate();
            }
            if (Wait(m_duration)) {
                if (m_onExit) {
                    m_onExit();
                }
                Finish();
            }
            break;
        default:
            Finish();
            break;
        }
    }
    bool IsRunning() const { return m_isRunning; }

    /// @brief ヒットストップのリクエストを行う
    void RequestHitStop(
        float duration,
        HitStopCallback onEntered = nullptr,
        HitStopCallback onUpdate = nullptr,
        HitStopCallback onExit = nullptr) {
        m_duration = duration;
        m_onEntered = onEntered;
        m_onUpdate = onUpdate;
        m_onExit = onExit;
        Start();
    }

    // 姿勢を呼び出し側で適用済みの場合、段階待ちなしで保持を開始する。
    void RequestHold(float duration) {
        RequestHitStop(duration);
        m_taskStep = 2;
        if (duration <= 0.0f) Finish();
    }

    /// @brief ヒットストップの残り時間を取得する
    float GetRemainingTime() const {
        if (!m_isRunning) return 0.0f;
        return m_duration > m_taskTimer ? m_duration - m_taskTimer : 0.0f;
    }
};