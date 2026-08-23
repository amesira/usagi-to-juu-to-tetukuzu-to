//===================================================
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_parameter.cpp
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのパラメータ編集パネルを管理するクラス
//===================================================
#include "mesh_effect_editor_parameter.h"

bool MeshEffectEditorParameter::Draw(MeshEffectDesc& desc)
{
    bool changed = false;
    // === 各モジュールの描画 ===
    changed |= DrawMain(desc.mainModule);
    changed |= DrawTransform(desc.transformModule);
    changed |= DrawFlipbook(desc.flipbookModule);
    changed |= DrawScroll(desc.scrollModule);
    changed |= DrawWave(desc.waveModule);
    changed |= DrawGradient(desc.gradientModule);
    changed |= DrawRenderer(desc.rendererModule);
    return changed;
}

#pragma region MeshEffectEditorのパラメータ編集パネルの内部描画関数
bool MeshEffectEditorParameter::DrawMain(MeshEffectData::MainModule& module)
{
    
}

bool MeshEffectEditorParameter::DrawTransform(MeshEffectData::TransformModule& module)
{
    
}

bool MeshEffectEditorParameter::DrawFlipbook(MeshEffectData::FlipbookModule& module)
{
    
}

bool MeshEffectEditorParameter::DrawScroll(MeshEffectData::ScrollModule& module)
{
    
}

bool MeshEffectEditorParameter::DrawWave(MeshEffectData::WaveModule& module)
{
    
}

bool MeshEffectEditorParameter::DrawGradient(MeshEffectData::GradientModule& module)
{
    
}

bool MeshEffectEditorParameter::DrawRenderer(MeshEffectData::RendererModule& module)
{
    
}