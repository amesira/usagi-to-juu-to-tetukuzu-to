#pragma once

#include "Engine/Asset/ParticleAsset/particle_system_asset.h"

class ParticleParameterPanel
{
public:
    bool Draw(ParticleSystemDesc& desc);

private:
    bool DrawMain(ParticleSystemData::MainModule& module);
    bool DrawEmission(ParticleSystemData::EmissionModule& module);
    bool DrawShape(ParticleSystemData::ShapeModule& module);
    bool DrawSizeOverLifetime(ParticleSystemData::SizeOverLifetimeModule& module);
    bool DrawTextureSheetAnimation(ParticleSystemData::TextureSheetAnimation& module);
    bool DrawRenderer(ParticleSystemData::RendererModule& module);

    bool DrawMinMaxFloat(const char* label, ParticleSystemData::MinMaxFloat& value);
    bool DrawMinMaxColor(const char* label, ParticleSystemData::MinMaxColor& value);
    bool DrawFloatCurve(const char* label, MiCurve::FloatCurve& curve);
};
