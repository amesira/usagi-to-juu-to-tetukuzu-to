//---------------------------------------------------
// File  ：Engine/Editor/ParticleEditor/particle_parameter_panel.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのパラメータ編集パネルを管理するクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_asset.h"

class ParticleParameterPanel {
public:
    /// @brief ParticleEditorのパラメータ編集パネルを描画する
    bool Draw(ParticleSystemDesc& desc);

private: // === ParticleEditorのパラメータ編集パネルの描画関数 ===
    bool DrawMain(ParticleSystemData::MainModule& module);
    bool DrawEmission(ParticleSystemData::EmissionModule& module);
    bool DrawShape(ParticleSystemData::ShapeModule& module);
    bool DrawSizeOverLifetime(ParticleSystemData::SizeOverLifetimeModule& module);
    bool DrawTextureSheetAnimation(ParticleSystemData::TextureSheetAnimation& module);
    bool DrawRenderer(ParticleSystemData::RendererModule& module);

};
