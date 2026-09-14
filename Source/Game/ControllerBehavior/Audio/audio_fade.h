#pragma once
#include <algorithm>
#include <cmath>
struct AudioFade {
    float value = 0, start = 0, target = 0, elapsed = 0, duration = 0;
    void Begin(float to, float seconds) {
        start = value; target = to; elapsed = 0;
        duration = std::isfinite(seconds) ? (std::max)(0.0f, seconds) : 0;
        if (!duration) value = target;
    }
    void Update(float dt) {
        if (!std::isfinite(dt) || dt < 0) return;
        elapsed = (std::min)(elapsed + dt, duration);
        value = duration > 0 ? start + (target - start) * (elapsed / duration) : target;
    }
    bool Finished() const { return elapsed >= duration; }
};
