//---------------------------------------------------
// File  ：Engine/Asset/Schema/enum_field_serializer.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・enum型のFieldSerializerの特殊化
//---------------------------------------------------
#pragma once
#include "field_serializer.h"
#include "enum_field_options.h"

namespace FieldSerialization
{
    using json = nlohmann::json;

    /// @brief enum型のFieldSerializerの特殊化
    template<class TEnum>
    class FieldSerializer<TEnum, EnumFieldOptions<TEnum>> {
    public:
        static json Serialize(const TEnum& value, const EnumFieldOptions<TEnum>& options)
        {
            for (const auto& choice : options.choices) {
                if (choice.value == value) {
                    return json(choice.key);
                }
            }

            return nullptr;
        }

        static bool Deserialize(const json& jsonValue, TEnum& value, const EnumFieldOptions<TEnum>& options)
        {
            if (!jsonValue.is_string()) {
                return false;
            }

            std::string key = jsonValue.get<std::string>();

            for (const auto& choice : options.choices) {
                if (key == choice.key) {
                    value = choice.value;
                    return true;
                }
            }

            return false;
        }

    };

}