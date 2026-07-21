//---------------------------------------------------
// File  ：Utility/mi_math_json.h
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・XMFLOAT3、XMFLOAT4のJSONシリアライズ/デシリアライズを提供する
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

#include <algorithm>
#include <nlohmann/json.hpp>

namespace MiMathJson
{
    using json = nlohmann::json;

    /// @brief XMFLOAT3をJSONにシリアライズする
    inline json SerializeFloat3(const XMFLOAT3& value)
    {
        return json::array({ value.x, value.y, value.z });
    }

    /// @brief XMFLOAT4をJSONにシリアライズする
    inline json SerializeFloat4(const XMFLOAT4& value)
    {
        return json::array({ value.x, value.y, value.z, value.w });
    }

    /// @brief JSONからXMFLOAT3をデシリアライズする
    inline bool DeserializeFloat3(const json& jsonValue, XMFLOAT3& outValue)
    {
        if (!jsonValue.is_array() || jsonValue.size() != 3)
        {
            return false;
        }
        outValue = {
            jsonValue[0].get<float>(),
            jsonValue[1].get<float>(),
            jsonValue[2].get<float>(),
        };
        return true;
    }

    /// @brief JSONからXMFLOAT4をデシリアライズする
    inline bool DeserializeFloat4(const json& jsonValue, XMFLOAT4& outValue)
    {
        if (!jsonValue.is_array() || jsonValue.size() != 4)
        {
            return false;
        }
        outValue = {
            jsonValue[0].get<float>(),
            jsonValue[1].get<float>(),
            jsonValue[2].get<float>(),
            jsonValue[3].get<float>(),
        };
        return true;
    }
}