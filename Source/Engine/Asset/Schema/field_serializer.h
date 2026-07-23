//---------------------------------------------------
// File  ：Engine/Asset/Schema/field_serializer.h
// Date  ：2026/07/23
// Author：Miu Kitamura
// 
// ・Fieldのシリアライズ・デシリアライズを行うための関数群
//---------------------------------------------------
#pragma once
#include "Utility/mi_math_json.h"
#include "Utility/mi_curve_json.h"

namespace FieldSerialization
{
    using json = nlohmann::json;

    /// @brief FieldSerializerのデフォルト実装（nlohmann::jsonのSerialize/Deserializeを使用）
    template<class T, class TOptions>
    class FieldSerializer {
    public:
        static json Serialize(const T& value, const TOptions& options)
        {
            return json(value);
        }

        static bool Deserialize(const json& jsonValue, T& value, const TOptions& options)
        {
            try
            {
                jsonValue.get_to(value);
                return true;
            }
            catch (const json::exception&)
            {
                return false;
            }
        }

    };

    /// @brief FieldSchemaの各Fieldをシリアライズする関数
    /// @param object 
    /// @param schema 
    template<class TObject, class TSchema>
    inline json SerializeFields(const TObject& object, const TSchema& schema) 
    {
        json schemaJson = json::object();

        schema.ForEach([&](const auto& field) {
            using TValue = std::decay_t<decltype(object.*(field.member))>;
            using TOptions = std::decay_t<decltype(field.options)>;
            const TValue& value = object.*(field.member);

            schemaJson[field.key] = FieldSerializer<TValue, TOptions>::Serialize(value, field.options);
            });

        return schemaJson;
    };

    /// @brief FieldSchemaの各Fieldをデシリアライズする関数
    /// @param object 
    /// @param schema 
    /// @return 全てのフィールドが正常にデシリアライズできた場合はtrue、1つでも失敗した場合はfalse
    /// MEMO: パラメータ追加、編集に柔軟に対応できるようにするため、JSONに存在しないフィールドはスキップしつつ、
    ///       処理自体は継続するようにする
    template<class TObject, class TSchema>
    inline bool DeserializeFields(const json& schemaJson, TObject& object, const TSchema& schema) 
    {
        if (!schemaJson.is_object())
        {
            return false;
        }

        bool allSucceeded = true;

        schema.ForEach([&](const auto& field) {
            using TValue = std::decay_t<decltype(object.*(field.member))>;
            using TOptions = std::decay_t<decltype(field.options)>;
            TValue& value = object.*(field.member);

            const auto it = schemaJson.find(field.key);

            // JSONにフィールドが存在しない場合はスキップ
            if (it == schemaJson.end()) {
                return ;
            }

            // === JSONの値を取得してデシリアライズ ===
            const json& jsonValue = it.value();
            TValue tempValue = value; // デシリアライズに失敗した場合に元の値を保持するための一時変数
            if (FieldSerializer<TValue, TOptions>::Deserialize(jsonValue, tempValue, field.options)) {
                value = std::move(tempValue); // デシリアライズに成功した場合のみ値を更新
            }
            else {
                allSucceeded = false;
            }
        });

        return allSucceeded;
    };

    /// @brief FieldSerializerのXMFLOAT2型の特殊化
    template<class TOptions>
    class FieldSerializer<XMFLOAT2, TOptions> {
    public:
        static json Serialize(const XMFLOAT2& value, const TOptions& options)
        {
            return MiMathJson::SerializeFloat2(value);
        }

        static bool Deserialize(const json& jsonValue, XMFLOAT2& value, const TOptions& options)
        {
            return MiMathJson::DeserializeFloat2(jsonValue, value);
        }

    };

    /// @brief FieldSerializerのXMFLOAT3型の特殊化
    template<class TOptions>
    class FieldSerializer<XMFLOAT3, TOptions> {
    public:
        static json Serialize(const XMFLOAT3& value, const TOptions& options)
        {
            return MiMathJson::SerializeFloat3(value);
        }

        static bool Deserialize(const json& jsonValue, XMFLOAT3& value, const TOptions& options)
        {
            return MiMathJson::DeserializeFloat3(jsonValue, value);
        }

    };

    /// @brief FieldSerializerのXMFLOAT4型の特殊化
    template<class TOptions>
    class FieldSerializer<XMFLOAT4, TOptions> {
    public:
        static json Serialize(const XMFLOAT4& value, const TOptions& options)
        {
            return MiMathJson::SerializeFloat4(value);
        }

        static bool Deserialize(const json& jsonValue, XMFLOAT4& value, const TOptions& options)
        {
            return MiMathJson::DeserializeFloat4(jsonValue, value);
        }

    };

    /// @brief FieldSerializerのFloatCurve型の特殊化
    template<class TOptions>
    class FieldSerializer<MiCurve::FloatCurve, TOptions> {
    public:
        static json Serialize(const MiCurve::FloatCurve& value, const TOptions& options)
        {
            return MiCurveJson::Serialize(value);
        }

        static bool Deserialize(const json& jsonValue, MiCurve::FloatCurve& value, const TOptions& options)
        {
            return MiCurveJson::Deserialize(jsonValue, value);
        }
    };
}

