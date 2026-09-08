// ui_shake_task.h
#pragma once
#include "Engine/Core/GamePlay/sequence_task.h"
#include <DirectXMath.h>
#include <cmath>

namespace PlayerUi {
class ShakeTask : public SequenceTask {
public:
    float m_intensity = 0.0f;
    float m_duration = 0.0f;
    float m_frequency = 20.0f;
    DirectX::XMFLOAT2 m_currentOffset = {};

    void Start() override {
        SequenceTask::Start();
        m_currentOffset = {};
        if (!(m_duration > 0.0f) || !std::isfinite(m_duration)
            || !(m_intensity > 0.0f) || !std::isfinite(m_intensity)
            || !(m_frequency > 0.0f) || !std::isfinite(m_frequency)) Finish();
    }

    void Update(float deltaTime) override {
        if (IsFinished() || !std::isfinite(deltaTime) || deltaTime < 0.0f) return;
        SequenceTask::Update(deltaTime);
        if (m_taskTimer >= m_duration) {
            m_currentOffset = {};
            Finish();
            return;
        }
        const float amplitude = m_intensity * (1.0f - m_taskTimer / m_duration);
        const float phase = m_taskTimer * m_frequency * DirectX::XM_2PI;
        m_currentOffset = { std::sin(phase) * amplitude, std::sin(phase * 1.3f) * amplitude };
    }

    void Cancel() {
        Reset();
        m_currentOffset = {};
    }
};
}
