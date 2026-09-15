//---------------------------------------------------
// File  ：_/Asset/EnvironmentAsset/environment_lighting_schema.h
// Date  ：2026/09/04
// Author：Miu Kitamura
// 
// ・環境ライト設定のスキーマ定義を行うヘッダファイル
//---------------------------------------------------
#pragma once
#include "environment_lighting_data.h"
#include "Engine/Asset/Schema/enum_field_options.h"
#include "Engine/Asset/Schema/field.h"
#include "Engine/Asset/Schema/field_schema.h"
#include "Engine/Asset/Schema/field_struct.h"

namespace EnvironmentLightingSchema
{
    inline const auto& GetDirectionalLightSchema()
    {
        using Module = DirectionalLightSettings;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("intensity", "Intensity", &Module::intensity,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("direction", "Direction", &Module::direction,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -1.0f, .maxValue = 1.0f }),
            MakeField("color", "Color", &Module::color, ColorFieldOptions{}),
            MakeField("ambientColor", "Ambient Color", &Module::ambientColor, ColorFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetRimLightSchema()
    {
        using Module = RimLightSettings;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("intensity", "Intensity", &Module::intensity,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("threshold", "Threshold", &Module::threshold,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
            MakeField("color", "Color", &Module::color, ColorFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetHemisphereLightSchema()
    {
        using Module = HemisphereLightSettings;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("intensity", "Intensity", &Module::intensity,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("skyColor", "Sky Color", &Module::skyColor, ColorFieldOptions{}),
            MakeField("groundColor", "Ground Color", &Module::groundColor, ColorFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetCubicColorLightSchema()
    {
        using Module = CubicColorLightSettings;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("intensity", "Intensity", &Module::intensity,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("upColor", "Up Color", &Module::upColor, ColorFieldOptions{}),
            MakeField("downColor", "Down Color", &Module::downColor, ColorFieldOptions{}),
            MakeField("leftColor", "Left Color", &Module::leftColor, ColorFieldOptions{}),
            MakeField("rightColor", "Right Color", &Module::rightColor, ColorFieldOptions{}),
            MakeField("frontColor", "Front Color", &Module::frontColor, ColorFieldOptions{}),
            MakeField("backColor", "Back Color", &Module::backColor, ColorFieldOptions{})
        };
        return schema;
    }

    inline const auto& GetLightingSettingsSchema()
    {
        using Module = EnvironmentLightingData;
        static const auto schema = FieldSchema{
            MakeStructField("directionalLight", "Directional Light", 
                &Module::directionalLight, GetDirectionalLightSchema(), DefaultFieldOptions{}),
            MakeStructField("rimLight", "Rim Light",
                &Module::rimLight, GetRimLightSchema(), DefaultFieldOptions{}),
            MakeStructField("hemisphereLight", "Hemisphere Light",
                &Module::hemisphereLight, GetHemisphereLightSchema(), DefaultFieldOptions{}),
            MakeStructField("cubicColorLight", "Cubic Color Light",
                &Module::cubicColorLight, GetCubicColorLightSchema(), DefaultFieldOptions{}),
        };
        return schema;
    }
}
