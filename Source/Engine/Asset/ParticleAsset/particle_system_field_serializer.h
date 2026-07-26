//---------------------------------------------------
// File  ：Engine/Asset/Schema/particle_system_field_serializer.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleSystemData特有のFieldSerializerの特殊化を定義する
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_data.h"
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
            return {
                { "randomBetweenTwoConstants", value.randomBetweenTwoConstants },
                { "constant", value.constant },
                { "constantMin", value.constantMin },
                { "constantMax", value.constantMax }
            };
        }

        static bool Deserialize(const json& jsonValue, ParticleSystemData::MinMaxFloat& value, const TOptions& options)
        {
            if (!jsonValue.is_object()) return false;

            auto temporary = value;
            try {
                temporary.randomBetweenTwoConstants = jsonValue.value(
                    "randomBetweenTwoConstants",
                    temporary.randomBetweenTwoConstants);
                temporary.constant = jsonValue.value("constant", temporary.constant);
                temporary.constantMin = jsonValue.value("constantMin", temporary.constantMin);
                temporary.constantMax = jsonValue.value("constantMax", temporary.constantMax);
            }
            catch (const json::exception&) {
                return false;
            }

            value = temporary;
            return true;
        }
    };

    /// @brief FieldSerializerのParticleSystemData::MinMaxColor型の特殊化
    template<class TOptions>
    class FieldSerializer<ParticleSystemData::MinMaxColor, TOptions> {
    public:
        static json Serialize(const ParticleSystemData::MinMaxColor& value, const TOptions& options)
        {
            return {
                { "randomBetweenTwoColors", value.randomBetweenTwoColors },
                { "color", MiMathJson::SerializeFloat4(value.color) },
                { "colorMin", MiMathJson::SerializeFloat4(value.colorMin) },
                { "colorMax", MiMathJson::SerializeFloat4(value.colorMax) }
            };
        }

        static bool Deserialize(const json& jsonValue, ParticleSystemData::MinMaxColor& value, const TOptions& options)
        {
            if (!jsonValue.is_object()) return false;

            auto temporary = value;
            try {
                temporary.randomBetweenTwoColors = jsonValue.value(
                    "randomBetweenTwoColors",
                    temporary.randomBetweenTwoColors);
            }
            catch (const json::exception&) {
                return false;
            }

            if (const auto it = jsonValue.find("color");
                it != jsonValue.end() &&
                !MiMathJson::DeserializeFloat4(*it, temporary.color)) {
                return false;
            }
            if (const auto it = jsonValue.find("colorMin");
                it != jsonValue.end() &&
                !MiMathJson::DeserializeFloat4(*it, temporary.colorMin)) {
                return false;
            }
            if (const auto it = jsonValue.find("colorMax");
                it != jsonValue.end() &&
                !MiMathJson::DeserializeFloat4(*it, temporary.colorMax)) {
                return false;
            }

            value = temporary;
            return true;
        }
    };
}
