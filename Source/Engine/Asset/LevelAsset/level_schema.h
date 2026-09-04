//----------------------------------------------------
// File  ：_/Asset/LevelAsset/level_schema.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//----------------------------------------------------
#pragma once

#include "level_data.h"
#include "Engine/Asset/Schema/field_master.h"

namespace LevelSchema
{
    inline const auto& GetTransformSchema()
    {
        using Module = LevelTransformData;
        static const auto schema = FieldSchema{
            MakeField("position", "Position", &Module::position),
            MakeField("rotationDegrees", "Rotation", &Module::rotationDegrees),
            MakeField("scale", "Scale", &Module::scale)
        };
        return schema;
    }

    inline const auto& GetColliderSchema()
    {
        using Module = LevelColliderData;
        static const EnumFieldOptions<LevelColliderType> typeOptions = { {
            { LevelColliderType::None, "None", "None" },
            { LevelColliderType::Box, "Box", "Box" },
            { LevelColliderType::Sphere, "Sphere", "Sphere" },
        } };
        static const auto schema = FieldSchema{
            MakeField("type", "Type", &Module::type, typeOptions),
            MakeField("center", "Center", &Module::center),
            MakeField("boxSize", "Box Size", &Module::boxSize),
            MakeField("sphereRadius", "Sphere Radius", &Module::sphereRadius)
        };
        return schema;
    }

    inline const auto& GetObjectSchema()
    {
        using Module = LevelObjectData;
        static const auto schema = FieldSchema{
            MakeField("id", "ID", &Module::id),
            MakeField("name", "Name", &Module::name),
            MakeStructField("transform", "Transform", &Module::transform,
                GetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("collider", "Collider", &Module::collider,
                GetColliderSchema(), DefaultFieldOptions{}),

            // モデルとマテリアルは検索用の文字列として保持
            MakeField("modelPath", "Model Path", &Module::modelPath),
            MakeField("materialNames", "Materials", &Module::materialNames)
        };
        return schema;
    }
}

// リストをシリアライズするためのFieldSerializerの特殊化
namespace FieldSerialization
{
    template<>
    class FieldSerializer<std::vector<LevelObjectData>, DefaultFieldOptions>
    {
    public:
        static json Serialize(const std::vector<LevelObjectData>& values, const DefaultFieldOptions&)
        {
            json result = json::array();
            for (const LevelObjectData& value : values)
                result.push_back(SerializeFields(value, LevelSchema::GetObjectSchema()));
            return result;
        }

        static bool Deserialize(
            const json& jsonValue,
            std::vector<LevelObjectData>& values,
            const DefaultFieldOptions&)
        {
            if (!jsonValue.is_array()) return false;
            std::vector<LevelObjectData> loaded;
            loaded.reserve(jsonValue.size());
            for (const json& item : jsonValue)
            {
                LevelObjectData value;
                if (!DeserializeFields(item, value, LevelSchema::GetObjectSchema())) return false;
                loaded.push_back(std::move(value));
            }
            values = std::move(loaded);
            return true;
        }
    };
}

namespace LevelSchema
{
    inline const auto& GetSchema()
    {
        using Module = LevelData;
        static const auto schema = FieldSchema{
            MakeField("objects", "Objects", &Module::objects)
        };
        return schema;
    }
}
