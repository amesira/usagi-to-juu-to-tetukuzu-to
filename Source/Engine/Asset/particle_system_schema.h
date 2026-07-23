#pragma once

#include "Engine/Asset/particle_system_data.h"
#include "Engine/Asset/Schema/enum_field_options.h"
#include "Engine/Asset/Schema/field.h"
#include "Engine/Asset/Schema/field_schema.h"

namespace ParticleSystemSchema
{
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
}
