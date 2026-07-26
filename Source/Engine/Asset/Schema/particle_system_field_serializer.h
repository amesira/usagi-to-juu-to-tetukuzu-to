//---------------------------------------------------
// File  ：Engine/Asset/Schema/particle_system_field_serializer.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleSystemData特有のFieldSerializerの特殊化を定義する
//---------------------------------------------------
#pragma once
#include "Engine/Asset/particle_system_data.h"
#include "Engine/Asset/Schema/field_serializer.h"

namespace FieldSerialization
{
    using json = nlohmann::json;

    /// @brief FieldSerializerのParticleSystemData::MinMaxFloat型の特殊化
    template<class TOptions>
    class FieldSerializer<ParticleSystemData::MinMaxFloat, TOptions> {
    public:
        static json Serialize(const ParticleSystemData::MinMaxFloat& value, const TOptions& options)
        {
            json jsonValue = json::object();

            jsonValue["randomBetweenTwoConstants"] = Serialize(value.randomBetweenTwoConstants, options);
            jsonValue["constant"] = Serialize(value.constant, options);
            jsonValue["constantMin"] = Serialize(value.constantMin, options);
            jsonValue["constantMax"] = Serialize(value.constantMax, options);

            return jsonValue;
        }

        static bool Deserialize(const json& jsonValue, ParticleSystemData::MinMaxFloat& value, const TOptions& options)
        {
            if (!jsonValue.is_object()) return false;

            value.randomBetweenTwoConstants = Deserialize(jsonValue.value("randomBetweenTwoConstants", value.constant), options);
            value.constant = Deserialize(jsonValue.value("constant", value.constant), options);
            value.constantMin = Deserialize(jsonValue.value("constantMin", value.constantMin), options);
            value.constantMax = Deserialize(jsonValue.value("constantMax", value.constantMax), options);

            return true;
        }
    };

    /// @brief FieldSerializerのParticleSystemData::MinMaxColor型の特殊化
    template<class TOptions>
    class FieldSerializer<ParticleSystemData::MinMaxColor, TOptions> {
    public:
        static json Serialize(const ParticleSystemData::MinMaxColor& value, const TOptions& options)
        {
            json jsonValue = json::object();

            jsonValue["randomBetweenTwoColors"] = Serialize(value.randomBetweenTwoColors, options);
            jsonValue["color"] = Serialize(value.color, options);
            jsonValue["colorMin"] = Serialize(value.colorMin, options);
            jsonValue["colorMax"] = Serialize(value.colorMax, options);

            return jsonValue;
        }

        static bool Deserialize(const json& jsonValue, ParticleSystemData::MinMaxColor& value, const TOptions& options)
        {
            if (!jsonValue.is_object()) return false;

            value.randomBetweenTwoColors = Deserialize(jsonValue.value("randomBetweenTwoColors", value.randomBetweenTwoColors), options);
            value.color = Deserialize(jsonValue.value("color", value.color), options);
            value.colorMin = Deserialize(jsonValue.value("colorMin", value.colorMin), options);
            value.colorMax = Deserialize(jsonValue.value("colorMax", value.colorMax), options);

            return true;
        }
    };
}