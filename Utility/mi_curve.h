#pragma once
#include <DirectXMath.h>
#include <vector>
#include <algorithm>

using namespace DirectX;

// カーブを扱う名前空間
namespace MiCurve {

    struct FloatCurveKey {
        float time = 0.0f;
        float value = 1.0f;
    };
    struct FloatCurve {
        std::vector<FloatCurveKey> keys = {
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        float Evaluate(float normalizedTime) const;
    };
    inline float Evaluate(const FloatCurve& curve, float normalizedTime)
    {
        if (curve.keys.empty()) return 1.0f;
        if (curve.keys.size() == 1) return curve.keys.front().value;

        const auto& keys = curve.keys;
        if (normalizedTime <= keys.front().time) return keys.front().value;
        if (normalizedTime >= keys.back().time) return keys.back().value;

        for (size_t i = 1; i < keys.size(); ++i) {
            if (normalizedTime > keys[i].time) continue;

            const auto& prev = keys[i - 1];
            const auto& next = keys[i];
            const float range = (next.time - prev.time) > 0.0001f
                ? (next.time - prev.time)
                : 0.0001f;
            const float t = (normalizedTime - prev.time) / range;
            return prev.value + (next.value - prev.value) * t;
        }

        return keys.back().value;
    }

    struct Float3CurveKey {
        float time = 0.0f;
        XMFLOAT3 value = { 1.0f, 1.0f, 1.0f };
    };
    struct Float3Curve {
        std::vector<Float3CurveKey> keys = {
            { 0.0f, {1.0f, 1.0f, 1.0f} },
            { 1.0f, {1.0f, 1.0f, 1.0f} },
        };
        XMFLOAT3 Evaluate(float normalizedTime) const;
    };
    inline XMFLOAT3 Evaluate(const Float3Curve& curve, float normalizedTime)
    {
        if (curve.keys.empty()) return { 1.0f, 1.0f, 1.0f };
        if (curve.keys.size() == 1) return curve.keys.front().value;
        const auto& keys = curve.keys;
        if (normalizedTime <= keys.front().time) return keys.front().value;
        if (normalizedTime >= keys.back().time) return keys.back().value;
        for (size_t i = 1; i < keys.size(); ++i) {
            if (normalizedTime > keys[i].time) continue;
            const auto& prev = keys[i - 1];
            const auto& next = keys[i];
            const float range = (next.time - prev.time) > 0.0001f
                ? (next.time - prev.time)
                : 0.0001f;
            const float t = (normalizedTime - prev.time) / range;
            XMFLOAT3 result;
            result.x = prev.value.x + (next.value.x - prev.value.x) * t;
            result.y = prev.value.y + (next.value.y - prev.value.y) * t;
            result.z = prev.value.z + (next.value.z - prev.value.z) * t;
            return result;
        }
        return keys.back().value;
    }

    struct Float4CurveKey {
        float time = 0.0f;
        XMFLOAT4 value = { 1.0f, 1.0f, 1.0f, 1.0f };
    };
    struct Float4Curve {
        std::vector<Float4CurveKey> keys = {
            { 0.0f, {1.0f, 1.0f, 1.0f, 1.0f} },
            { 1.0f, {1.0f, 1.0f, 1.0f, 1.0f} },
        };
        XMFLOAT4 Evaluate(float normalizedTime) const;
    };
    inline XMFLOAT4 Evaluate(const Float4Curve& curve, float normalizedTime)
    {
        if (curve.keys.empty()) return { 1.0f, 1.0f, 1.0f, 1.0f };
        if (curve.keys.size() == 1) return curve.keys.front().value;
        const auto& keys = curve.keys;
        if (normalizedTime <= keys.front().time) return keys.front().value;
        if (normalizedTime >= keys.back().time) return keys.back().value;
        for (size_t i = 1; i < keys.size(); ++i) {
            if (normalizedTime > keys[i].time) continue;
            const auto& prev = keys[i - 1];
            const auto& next = keys[i];
            const float range = (next.time - prev.time) > 0.0001f
                ? (next.time - prev.time)
                : 0.0001f;
            const float t = (normalizedTime - prev.time) / range;
            XMFLOAT4 result;
            result.x = prev.value.x + (next.value.x - prev.value.x) * t;
            result.y = prev.value.y + (next.value.y - prev.value.y) * t;
            result.z = prev.value.z + (next.value.z - prev.value.z) * t;
            result.w = prev.value.w + (next.value.w - prev.value.w) * t;
            return result;
        }
        return keys.back().value;
    }
    

    inline float FloatCurve::Evaluate(float normalizedTime) const
    {
        return MiCurve::Evaluate(*this, normalizedTime);
    }

    inline XMFLOAT3 Float3Curve::Evaluate(float normalizedTime) const
    {
        return MiCurve::Evaluate(*this, normalizedTime);
    }

    inline XMFLOAT4 Float4Curve::Evaluate(float normalizedTime) const
    {
        return MiCurve::Evaluate(*this, normalizedTime);
    }
}
