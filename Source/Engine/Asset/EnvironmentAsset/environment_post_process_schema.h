//---------------------------------------------------
// File  ：_/Asset/EnvironmentAsset/environment_post_process_schema.h
// Date  ：2026/09/04
// Author：Miu Kitamura
// 
// ・ポストプロセス設定のスキーマ定義を行うヘッダファイル
//---------------------------------------------------
#pragma once
#include "environment_post_process_data.h"

#include "Engine/Asset/Schema/field.h"
#include "Engine/Asset/Schema/field_schema.h"
#include "Engine/Asset/Schema/field_struct.h"
#include "Engine/Asset/Schema/enum_field_options.h"

namespace EnvironmentPostProcessSchema
{
    inline const auto& GetBloomSchema()
    {
        using Module = BloomSettings;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("intensity", "Intensity", &Module::intensity,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("threshold", "Threshold", &Module::threshold,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10.0f })
        };
        return schema;
    }

    inline const auto& GetPostProcessSchema()
    {
        using Module = EnvironmentPostProcessData;
        static const auto schema = FieldSchema{
            MakeStructField("bloom", "Bloom", &Module::bloom, GetBloomSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}