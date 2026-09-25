#pragma once

#include <algorithm>
#include <cmath>
#include <DirectXMath.h>

#include "Engine/Core/GamePlay/sequence_task.h"
#include "Utility/mi_signal.h"

/// @brief 近距離攻撃の予備動作中に使用する位置シェイクタスク。
class EnemyMeleeAttackShakeTask : public SequenceTask {
public:
    float m_duration = 0.0f;
    float m_magnitude = 0.0f;
    float m_frequency = 25.0f;
    DirectX::XMFLOAT3 m_axis = { 1.0f, 0.35f, 1.0f };
    DirectX::XMFLOAT3 m_offset = {};

    void Start() override
    {
        SequenceTask::Start();
        m_offset = {};
        if (!(m_duration > 0.0f) || !std::isfinite(m_duration)
            || !(m_magnitude > 0.0f) || !std::isfinite(m_magnitude)
            || !(m_frequency > 0.0f) || !std::isfinite(m_frequency)) {
            Finish();
        }
    }

    void Update(float deltaTime) override
    {
        if (IsFinished() || !std::isfinite(deltaTime) || deltaTime < 0.0f) return;

        SequenceTask::Update(deltaTime);
        if (m_taskTimer >= m_duration) {
            m_offset = {};
            Finish();
            return;
        }

        // 攻撃直前ほど大きく震えるよう、振幅を二次曲線で増加させる。
        const float progress = (std::clamp)(m_taskTimer / m_duration, 0.0f, 1.0f);
        const float amplitude = m_magnitude * progress * progress;
        const float noiseTime = m_taskTimer * m_frequency;

        m_offset.x = MiSignal::Perlin1D(noiseTime) * amplitude * m_axis.x;
        m_offset.y = MiSignal::Perlin1D(noiseTime + 100.0f) * amplitude * m_axis.y;
        m_offset.z = MiSignal::Perlin1D(noiseTime + 200.0f) * amplitude * m_axis.z;
    }

    void Cancel()
    {
        Reset();
        m_offset = {};
    }
};
