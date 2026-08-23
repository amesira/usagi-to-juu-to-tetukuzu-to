//===================================================
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_parameter.cpp
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのパラメータ編集パネルを管理するクラス
//===================================================
#include "mesh_effect_editor_parameter.h"

#include "Engine/Editor/i_editor_window.h"
#include "Engine/Editor/Schema/enum_field_editor.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Asset/MeshEffectAsset/mesh_effect_schema.h"

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
    if (!ImGui::CollapsingHeader("Main", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetMainSchema());
}

bool MeshEffectEditorParameter::DrawTransform(MeshEffectData::TransformModule& module)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetTransformSchema());
}

bool MeshEffectEditorParameter::DrawFlipbook(MeshEffectData::FlipbookModule& module)
{
    if (!ImGui::CollapsingHeader("Flipbook", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetFlipbookSchema());
}

bool MeshEffectEditorParameter::DrawScroll(MeshEffectData::ScrollModule& module)
{
    if (!ImGui::CollapsingHeader("Scroll")) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetScrollSchema());
}

bool MeshEffectEditorParameter::DrawWave(MeshEffectData::WaveModule& module)
{
    if (!ImGui::CollapsingHeader("Wave")) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetWaveSchema());
}

bool MeshEffectEditorParameter::DrawGradient(MeshEffectData::GradientModule& module)
{
    if (!ImGui::CollapsingHeader("Gradient")) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetGradientSchema());
}

bool MeshEffectEditorParameter::DrawRenderer(MeshEffectData::RendererModule& module)
{
    if (!ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(module, MeshEffectSchema::GetRendererSchema());
}
