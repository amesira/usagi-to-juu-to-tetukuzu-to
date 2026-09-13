#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

struct TitleCameraPose {
    DirectX::XMFLOAT3 position = {}, lookAt = {};
    float fov = 60;
};
class TitleCameraBlend {
    TitleCameraPose m_start;
    float m_elapsed = 0, m_duration = 0;
public:
    void Start(const TitleCameraPose& start, float duration) {
        m_start = start; m_elapsed = 0;
        m_duration = std::isfinite(duration) ? (std::max)(duration, 0.0f) : 0;
    }
    TitleCameraPose Update(const TitleCameraPose& target, float deltaTime) {
        if (std::isfinite(deltaTime) && deltaTime > 0) m_elapsed = (std::min)(m_elapsed + deltaTime, m_duration);
        if (IsComplete()) return target;
        float t = m_elapsed / m_duration;
        t = t * t * (3 - 2 * t);
        auto lerp = [t](DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b) {
            return DirectX::XMFLOAT3{a.x + (b.x-a.x)*t, a.y + (b.y-a.y)*t, a.z + (b.z-a.z)*t};
        };
        return {lerp(m_start.position,target.position), lerp(m_start.lookAt,target.lookAt), m_start.fov + (target.fov-m_start.fov)*t};
    }
    bool IsComplete() const { return m_elapsed >= m_duration; }
};
