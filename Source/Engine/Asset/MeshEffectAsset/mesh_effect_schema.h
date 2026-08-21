//---------------------------------------------------
// File  ：_/Asset/MeshEffectAsset/mesh_effect_schema.h
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffectのスキーマ
//---------------------------------------------------
#pragma once
#include "Engine/Asset/MeshEffectAsset/mesh_effect_data.h"
#include "Engine/Asset/Schema/enum_field_options.h"
#include "Engine/Asset/Schema/field.h"
#include "Engine/Asset/Schema/field_schema.h"
#include "Engine/Asset/Schema/field_struct.h"

namespace MeshEffectSchema 
{
#pragma region MeshEffectのenum型のFieldOptions定義

    inline const EnumFieldOptions<MeshEffectData::TimeMode>& GetTimeModeOptions()
    {
        static const EnumFieldOptions<MeshEffectData::TimeMode> options{
            {
                { MeshEffectData::TimeMode::Lifetime, "Lifetime", "Lifetime" },
                { MeshEffectData::TimeMode::Speed, "Speed", "Speed" }
            }
        };
        return options;
    }

    inline const EnumFieldOptions<MeshEffectData::BillboardMode>& GetBillboardModeOptions()
    {
        static const EnumFieldOptions<MeshEffectData::BillboardMode> options{
            {
                { MeshEffectData::BillboardMode::View, "View", "View" },
                { MeshEffectData::BillboardMode::Horizontal, "Horizontal", "Horizontal" }
            }
        };
        return options;
    }

    inline const EnumFieldOptions<MeshEffectData::BlendMode>& GetBlendModeOptions()
    {
        static const EnumFieldOptions<MeshEffectData::BlendMode> options{
            {
                { MeshEffectData::BlendMode::AlphaBlend, "AlphaBlend", "Alpha Blend" },
                { MeshEffectData::BlendMode::Additive, "Additive", "Additive" }
            }
        };
        return options;
    }

    inline const EnumFieldOptions<MeshEffectData::FlipbookPlaybackMode>& GetFlipbookPlaybackModeOptions()
    {
        static const EnumFieldOptions<MeshEffectData::FlipbookPlaybackMode> options{
            {
                { MeshEffectData::FlipbookPlaybackMode::Once, "Once", "Once" },
                { MeshEffectData::FlipbookPlaybackMode::Loop, "Loop", "Loop" },
                { MeshEffectData::FlipbookPlaybackMode::PingPong, "PingPong", "Ping Pong" }
            }
        };
        return options;
    }

    inline const EnumFieldOptions<MeshEffectData::WaveType>& GetWaveTypeOptions()
    {
        static const EnumFieldOptions<MeshEffectData::WaveType> options{
            {
                { MeshEffectData::WaveType::Vertex, "Vertex", "Vertex" },
                { MeshEffectData::WaveType::UV, "UV", "UV" }
            }
        };
        return options;
    }

#pragma endregion

    inline const auto& GetMainSchema()
    {
        using Module = MeshEffectData::MainModule;
        static const auto schema = FieldSchema{
            MakeField("duration", "Duration", &Module::duration,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("loop", "Loop", &Module::loop),
            MakeField("playOnAwake", "Play On Awake", &Module::playOnAwake),
            MakeField("simulationSpeed", "Simulation Speed", &Module::simulationSpeed,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 0.0f })
        };
        return schema;
    }

    inline const auto& GetTransformSchema()
    {
        using Module = MeshEffectData::TransformModule;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("scaleOverLifetime", "Scale Over Lifetime", &Module::scaleOverLifetime),
            MakeField("rotationOverLifetime", "Rotation Over Lifetime", &Module::rotationOverLifetime)
        };
        return schema;
    }

    inline const auto& GetFlipbookSchema()
    {
        using Module = MeshEffectData::FlipbookModule;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("tileX", "Tiles X", &Module::tileX,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 1.0f, .maxValue = 0.0f }),
            MakeField("tileY", "Tiles Y", &Module::tileY,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 1.0f, .maxValue = 0.0f }),
            MakeField("startFrame", "Start Frame", &Module::startFrame,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("frameCount", "Frame Count", &Module::frameCount,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 1.0f, .maxValue = 0.0f }),
            MakeField("timeMode", "Time Mode", &Module::timeMode, GetTimeModeOptions()),
            MakeField("framePerSecond", "Frames Per Second", &Module::framePerSecond,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("playbackMode", "Playback Mode", &Module::playbackMode, GetFlipbookPlaybackModeOptions())
        };
        return schema;
    }

    inline const auto& GetScrollSchema()
    {
        using Module = MeshEffectData::ScrollModule;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("tiling", "Tiling", &Module::tiling,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("offset", "Offset", &Module::offset,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("scrollSpeed", "Scroll Speed", &Module::scrollSpeed,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 0.0f })
        };
        return schema;
    }

    inline const auto& GetWaveSchema()
    {
        using Module = MeshEffectData::WaveModule;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("type", "Wave Type", &Module::type, GetWaveTypeOptions()),
            MakeField("direction", "Direction", &Module::direction,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("amplitudeOverLifetime", "Amplitude Over Lifetime", &Module::amplitudeOverLifetime),
            MakeField("frequency", "Frequency", &Module::frequency,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 0.0f }),
            MakeField("speed", "Speed", &Module::speed,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 0.0f })
        };
        return schema;
    }

    inline const auto& GetGradientSchema()
    {
        using Module = MeshEffectData::GradientModule;
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &Module::enabled),
            MakeField("color", "Color", &Module::color)
        };
        return schema;
    }

    inline const auto& GetRendererSchema()
    {
        using Module = MeshEffectData::RendererModule;
        static const auto schema = FieldSchema{
            MakeField("meshPath", "Mesh Path", &Module::meshPath),
            MakeField("materialPath", "Material Path", &Module::materialPath),
            MakeField("uvRect", "UV Rect", &Module::uvRect,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
            MakeField("billboardMode", "Billboard Mode", &Module::billboardMode, GetBillboardModeOptions()),
            MakeField("blendMode", "Blend Mode", &Module::blendMode, GetBlendModeOptions())
        };
        return schema;
    }
}
