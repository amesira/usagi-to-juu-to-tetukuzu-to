#pragma once
#include <DirectXMath.h>
#include <vector>
#include <algorithm>

using namespace DirectX;

// カーブを扱う名前空間
namespace MiCurve {

    struct CurveKey {
        float time = 0.0f;
        float value = 1.0f;
    };

    struct FloatCurve {
        std::vector<CurveKey> keys = {
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        float Evaluate(float normalizedTime) const;
    };

    struct Float3Curve {
        FloatCurve x;
        FloatCurve y;
        FloatCurve z;
        XMFLOAT3 Evaluate(float normalizedTime) const;

        Float3Curve(float value = 1.0f)
            : x({ {0.0f, value}, {1.0f, value} }),
              y({ {0.0f, value}, {1.0f, value} }),
              z({ {0.0f, value}, {1.0f, value} })
        {
        }
    };

    struct ColorCurve {
        FloatCurve r;
        FloatCurve g;
        FloatCurve b;
        FloatCurve a;
        XMFLOAT4 Evaluate(float normalizedTime) const;
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

    inline float FloatCurve::Evaluate(float normalizedTime) const
    {
        return MiCurve::Evaluate(*this, normalizedTime);
    }

    inline XMFLOAT3 Float3Curve::Evaluate(float normalizedTime) const
    {
        return XMFLOAT3{
            x.Evaluate(normalizedTime),
            y.Evaluate(normalizedTime),
            z.Evaluate(normalizedTime)
        };
    }

    inline XMFLOAT4 ColorCurve::Evaluate(float normalizedTime) const
    {
        return XMFLOAT4{
            r.Evaluate(normalizedTime),
            g.Evaluate(normalizedTime),
            b.Evaluate(normalizedTime),
            a.Evaluate(normalizedTime)
        };
    }

}
