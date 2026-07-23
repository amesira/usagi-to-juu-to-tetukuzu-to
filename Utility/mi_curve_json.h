//---------------------------------------------------
// File  ：Utility/mi_curve_json.h
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・MiCurveのJSONシリアライズ/デシリアライズを提供する
//---------------------------------------------------
#pragma once
#include "Utility/mi_curve.h"

#include <algorithm>
#include <nlohmann/json.hpp>

namespace MiCurveJson
{
    using json = nlohmann::json;

    /// @brief MiCurve::FloatCurveをJSONにシリアライズする
    inline json Serialize(const MiCurve::FloatCurve& curve)
    {
        json keys = json::array();

        for (const MiCurve::CurveKey& key : curve.keys)
        {
            keys.push_back({
                { "time", key.time },
                { "value", key.value },
            });
        }

        return json{ { "keys", std::move(keys) } };
    }

    /// @brief JSONからMiCurve::FloatCurveをデシリアライズする
    inline bool Deserialize(const json& jsonValue, MiCurve::FloatCurve& outCurve)
    {
        if (!jsonValue.is_object())
        {
            return false;
        }

        // "keys"フィールドが存在し、配列であることを確認
        const auto keysIt = jsonValue.find("keys");
        if (keysIt == jsonValue.end() || !keysIt->is_array())
        {
            return false;
        }

        MiCurve::FloatCurve loadedCurve;
        loadedCurve.keys.clear();
        loadedCurve.keys.reserve(keysIt->size());

        // === 各キーをデシリアライズ ===
        for (const json& keyJson : *keysIt)
        {
            if (!keyJson.is_object())
            {
                return false;
            }

            const auto timeIt = keyJson.find("time");
            const auto valueIt = keyJson.find("value");
            if (timeIt == keyJson.end() || !timeIt->is_number() ||
                valueIt == keyJson.end() || !valueIt->is_number())
            {
                return false;
            }

            loadedCurve.keys.push_back({
                timeIt->get<float>(),
                valueIt->get<float>(),
            });
        }

        // キーを時間順にソートする
        std::stable_sort(
            loadedCurve.keys.begin(),
            loadedCurve.keys.end(),
            [](const MiCurve::CurveKey& lhs, const MiCurve::CurveKey& rhs)
            {
                return lhs.time < rhs.time;
            });

        outCurve = std::move(loadedCurve);
        return true;
    }
}
