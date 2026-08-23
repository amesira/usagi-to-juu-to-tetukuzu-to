//---------------------------------------------------
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_parameter.h
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのパラメータ編集パネルを管理するクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/MeshEffectAsset/mesh_effect_data.h"

class MeshEffectEditorParameter {
public:
    /// @brief MeshEffectEditorのパラメータ編集パネルを描画する
    bool Draw(MeshEffectDesc& desc);

public:
    // === MeshEffectEditorのパラメータ編集パネルの描画関数 ===
    bool DrawMain(MeshEffectData::MainModule& module);
    bool DrawTransform(MeshEffectData::TransformModule& module);
    bool DrawFlipbook(MeshEffectData::FlipbookModule& module);
    bool DrawScroll(MeshEffectData::ScrollModule& module);
    bool DrawWave(MeshEffectData::WaveModule& module);
    bool DrawGradient(MeshEffectData::GradientModule& module);
    bool DrawRenderer(MeshEffectData::RendererModule& module);

};