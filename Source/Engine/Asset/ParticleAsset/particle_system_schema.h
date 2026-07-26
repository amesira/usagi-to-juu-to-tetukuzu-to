//---------------------------------------------------
// File  ：Source/Engine/Asset/particle_system_schema.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleSystemDataの各モジュールのスキーマを定義する
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_data.h"
#include "Engine/Asset/Schema/enum_field_options.h"
#include "Engine/Asset/Schema/field.h"
#include "Engine/Asset/Schema/field_schema.h"
#include "Engine/Asset/Schema/field_struct.h"

namespace ParticleSystemSchema
{
#pragma region ParticleSystemのenum型のFieldOptions定義

    /// @brief ParticleSystemData::SimulationSpaceのFieldOptionsを取得
    inline const EnumFieldOptions<ParticleSystemData::SimulationSpace>& GetSimulationSpaceOptions()
    {
        static const EnumFieldOptions<ParticleSystemData::SimulationSpace> options{
            {
                {
                    ParticleSystemData::SimulationSpace::Local,
                    "Local",
                    "Local"
                },
                {
                    ParticleSystemData::SimulationSpace::World,
                    "World",
                    "World"
                }
            }
        };
        return options;
    }

    /// @brief ParticleSystemData::ShapeTypeのFieldOptionsを取得
    inline const EnumFieldOptions<ParticleSystemData::ShapeType>& GetShapeTypeOptions()
    {
        static const EnumFieldOptions<ParticleSystemData::ShapeType> options{
            {
                {
                    ParticleSystemData::ShapeType::Sphere,
                    "Sphere",
                    "Sphere"
                },
                {
                    ParticleSystemData::ShapeType::Cone,
                    "Cone",
                    "Cone"
                }
            }
        };
        return options;
    }

    /// @brief ParticleSystemData::BillboardModeのFieldOptionsを取得
    inline const EnumFieldOptions<ParticleSystemData::BillboardMode>& GetBillboardMode()
    {
        static const EnumFieldOptions<ParticleSystemData::BillboardMode> options{
            {
                {
                    ParticleSystemData::BillboardMode::View,
                    "View",
                    "View"
                },
                {
                    ParticleSystemData::BillboardMode::Horizontal,
                    "Horizontal",
                    "Horizontal"
                }
            }
        };
        return options;
    }

    /// @brief ParticleSystemData::BlendModeのFieldOptionsを取得
    inline const EnumFieldOptions<ParticleSystemData::BlendMode>& GetBlendMode() 
    {
        static const EnumFieldOptions<ParticleSystemData::BlendMode> options{
            {
                {
                    ParticleSystemData::BlendMode::AlphaBlend,
                    "AlphaBlend",
                    "AlphaBlend"
                },
                {
                    ParticleSystemData::BlendMode::Additive,
                    "Additive",
                    "Additive"
                }
            }
        };
        return options;
    }

    /// @brief ParticleSystemData::TimeModeのFieldOptionsを取得
    inline const EnumFieldOptions<ParticleSystemData::TimeMode>& GetTimeModeOptions()
    {
        static const EnumFieldOptions<ParticleSystemData::TimeMode> options{
            {
                {
                    ParticleSystemData::TimeMode::Lifetime,
                    "Lifetime",
                    "Lifetime"
                },
                {
                    ParticleSystemData::TimeMode::Speed,
                    "Speed",
                    "Speed"
                }
            }
        };
        return options;
    }


#pragma endregion

    /// @brief MainModuleのスキーマを取得
    inline const auto& GetMainSchema()
    {
        using Module = ParticleSystemData::MainModule;

        static const auto schema = FieldSchema{
            MakeField(
                "duration",
                "Duration",
                &Module::duration,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField("loop", "Loop", &Module::loop),
            MakeField("playOnAwake", "Play On Awake", &Module::playOnAwake),

            // MinMax
            MakeField(
                "startLifetime",
                "Start Lifetime",
                &Module::startLifetime,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "startSpeed",
                "Start Speed",
                &Module::startSpeed,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "startSize",
                "Start Size",
                &Module::startSize,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField("startColor", "Start Color", &Module::startColor, ColorFieldOptions{}),

            MakeField(
                "gravity",
                "Gravity",
                &Module::gravity,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "simulationSpeed",
                "Simulation Speed",
                &Module::simulationSpeed,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "simulationSpace",
                "Simulation Space",
                &Module::simulationSpace,
                GetSimulationSpaceOptions())
        };

        return schema;
    }

    /// @brief EmissionModuleのスキーマを取得
    inline const auto& GetEmissionSchema()
    {
        using Module = ParticleSystemData::EmissionModule;

        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField(
                "rateOverTime",
                "Rate Over Time",
                &Module::rateOverTime,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "rateOverDistance",
                "Rate Over Distance",
                &Module::rateOverDistance,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f })
        };

        return schema;
    }

    /// @brief ShapeModuleのスキーマを取得
    inline const auto& GetShapeSchema()
    {
        using Module = ParticleSystemData::ShapeModule;

        // SphereShapeスキーマ
        static const auto sphereShapeSchema = FieldSchema{
            MakeField(
                "radius",
                "Radius",
                &Module::SphereShape::radius,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField("emitFromShell", "Emit From Shell", &Module::SphereShape::emitFromShell)
        };

        // ConeShapeスキーマ
        static const auto coneShapeSchema = FieldSchema{
            MakeField(
                "angle",
                "Angle",
                &Module::ConeShape::angle,
                AngleFieldOptions{
                    .minValue = 0.0f,
                    .maxValue = 360.0f }),
            MakeField(
                "radius",
                "Radius",
                &Module::ConeShape::radius,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "length",
                "Length",
                &Module::ConeShape::length,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
                    MakeField("emitFromBase", "Emit From Base", &Module::ConeShape::emitFromBase)
        };

        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField(
                "type",
                "Shape Type",
                &Module::shapeType,
                GetShapeTypeOptions()),

            // SphereShapeの設定
            MakeStructField(
                "sphere",
                "Sphere Shape",
                &Module::sphere,
                sphereShapeSchema,
                DefaultFieldOptions{}),
            // ConeShapeの設定
            MakeStructField(
                "cone",
                "Cone Shape",
                &Module::cone,
                coneShapeSchema,
                DefaultFieldOptions{}),
            
            MakeField(
                "randomDirectionAmount",
                "Random Direction Amount",
                &Module::randomDirectionAmount,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 1.0f })
        };

        return schema;
    }

    /// @brief SizeOverLifetimeModuleのスキーマを取得
    inline const auto& GetSizeOverLifetimeSchema()
    {
        using Module = ParticleSystemData::SizeOverLifetimeModule;

        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("size", "Size Curve", &Module::size)
        };

        return schema;
    }

    /// @brief TextureSheetAnimationModuleのスキーマを取得
    inline const auto& GetTextureSheetAnimationSchema()
    {
        using Module = ParticleSystemData::TextureSheetAnimation;

        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField(
                "tileX",
                "Tiles X",
                &Module::tileX,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 1.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "tileY",
                "Tiles Y",
                &Module::tileY,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 1.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "startFrame",
                "Start Frame",
                &Module::startFrame,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "frameCount",
                "Frame Count",
                &Module::frameCount,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 1.0f,
                    .maxValue = 0.0f }),
            MakeField(
                "timeMode",
                "Time Mode",
                &Module::timeMode,
                GetTimeModeOptions()),
            MakeField(
                "framePerSecond",
                "Frames Per Second",
                &Module::framePerSecond,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 0.0f }),
            MakeField("loop", "Loop", &Module::loop)
        };

        return schema;
    }

    /// @brief RendererModuleのスキーマを取得
    inline const auto& GetRendererSchema()
    {
        using Module = ParticleSystemData::RendererModule;
        static const auto schema = FieldSchema{
            MakeField("texturePath", "Texture Path", &Module::texturePath),
            MakeField(
                "uvRect", 
                "UV Rect", 
                &Module::uvRect,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 1.0f }),
            MakeField(
                "billboardMode",
                "Billboard Mode",
                &Module::billboardMode,
                GetBillboardMode()),
            MakeField(
                "blendMode",
                "Blend Mode",
                &Module::blendMode,
                GetBlendMode()),
            MakeField("sortByDistance", "Sort By Distance", &Module::sortByDistance)
        };

        return schema;
    }

}
