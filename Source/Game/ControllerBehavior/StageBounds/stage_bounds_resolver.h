#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
namespace StageBounds {
struct Settings {
    DirectX::XMFLOAT3 min = {-50, -10, -50};
    DirectX::XMFLOAT3 max = {50, 30, 50};
    bool enableMinX = true, enableMaxX = true;
    bool enableMinY = true, enableMaxY = true;
    bool enableMinZ = true, enableMaxZ = true;
};
struct Shape {
    DirectX::XMFLOAT3 offset = {};
    DirectX::XMFLOAT3 extent = {};
};
inline void Sanitize(Settings& s) {
    auto axis = [](float& lo, float& hi, float defaultLo, float defaultHi) {
        if (!std::isfinite(lo)) lo = defaultLo;
        if (!std::isfinite(hi)) hi = defaultHi;
        if (lo > hi) std::swap(lo, hi);
    };
    axis(s.min.x, s.max.x, -50, 50);
    axis(s.min.y, s.max.y, -10, 30);
    axis(s.min.z, s.max.z, -50, 50);
}
// Normals point inward; used to remove only velocity directed out of bounds.
struct Result {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal = {};
    bool floor = false;
    void ClipVelocity(DirectX::XMFLOAT3& v) const {
        auto clip = [](float& value, float n) {
            if (n == 2 || value * n < 0) value = 0;
        };
        clip(v.x, normal.x); clip(v.y, normal.y); clip(v.z, normal.z);
    }
};
inline Result Resolve(const Settings& settings, const Shape& shape, DirectX::XMFLOAT3 position) {
    auto s = settings; Sanitize(s);
    Result r{position};
    auto axis = [](float& p, float& normal, float lo, float hi, float offset, float extent, bool lower, bool upper) {
        lo += extent - offset; hi -= extent + offset;
        if (lower && upper && lo > hi) { p = (lo + hi) * .5f; normal = 2; return; }
        if (lower && p <= lo) { p = lo; normal = 1; }
        if (upper && p >= hi) { p = hi; normal = -1; }
    };
    axis(r.position.x, r.normal.x, s.min.x, s.max.x, shape.offset.x, shape.extent.x, s.enableMinX, s.enableMaxX);
    axis(r.position.y, r.normal.y, s.min.y, s.max.y, shape.offset.y, shape.extent.y, s.enableMinY, s.enableMaxY);
    axis(r.position.z, r.normal.z, s.min.z, s.max.z, shape.offset.z, shape.extent.z, s.enableMinZ, s.enableMaxZ);
    r.floor = s.enableMinY && r.normal.y > 0;
    return r;
}
// Account for integration without changing PhysicsProcessor or Rigidbody parameters.
inline DirectX::XMFLOAT3 ConstrainVelocity(const Settings& settings, const Shape& shape,
    DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 velocity, DirectX::XMFLOAT3 friction, float gravityAcceleration, float dt) {
    if (!std::isfinite(dt) || dt <= 0) return velocity;
    DirectX::XMFLOAT3 integrated = {velocity.x * friction.x, velocity.y * friction.y + gravityAcceleration * dt, velocity.z * friction.z};
    DirectX::XMFLOAT3 predicted = {position.x + integrated.x * dt, position.y + integrated.y * dt, position.z + integrated.z * dt};
    const auto result = Resolve(settings, shape, predicted);
    auto correct = [dt](float& v, float actual, float desired, float f) {
        if (actual != desired && std::abs(f) > .000001f) v += (desired - actual) / (dt * f);
    };
    correct(velocity.x, predicted.x, result.position.x, friction.x);
    correct(velocity.y, predicted.y, result.position.y, friction.y);
    correct(velocity.z, predicted.z, result.position.z, friction.z);
    return velocity;
}
inline bool IsGrounded(const Settings& settings, const Shape& shape, const DirectX::XMFLOAT3& position, float distance) {
    auto s = settings; Sanitize(s);
    return s.enableMinY && position.y + shape.offset.y - shape.extent.y <= s.min.y + (std::max)(0.0f, distance);
}
}
