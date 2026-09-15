//===================================================
// File  ：Engine/Editor/ParticleEditor/particle_parameter_panel.cpp
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのパラメータ編集パネルを管理するクラス
//===================================================
#include "particle_parameter_panel.h"
#include "Engine/Editor/i_editor_window.h"

#include "Engine/Editor/Schema/enum_field_editor.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Asset/ParticleAsset/particle_system_schema.h"

#include "particle_system_field_editor.h"

/// @brief ParticleEditorのパラメータ編集パネルを描画する
bool ParticleParameterPanel::Draw(ParticleSystemDesc& desc)
{
    bool changed = false;
    changed |= DrawMain(desc.mainModule);
    changed |= DrawEmission(desc.emissionModule);
    changed |= DrawShape(desc.shapeModule);
    changed |= DrawSizeOverLifetime(desc.sizeOverLifetimeModule);
    changed |= DrawTextureSheetAnimation(desc.textureSheetAnimation);
    changed |= DrawRenderer(desc.rendererModule);
    return changed;
}

#pragma region ParticleEditorのパラメータ編集パネルの描画関数

bool ParticleParameterPanel::DrawMain(ParticleSystemData::MainModule& module)
{
    if (!ImGui::CollapsingHeader("Main", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetMainSchema());
}

bool ParticleParameterPanel::DrawEmission(ParticleSystemData::EmissionModule& module)
{
    if (!ImGui::CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetEmissionSchema());
}

bool ParticleParameterPanel::DrawShape(ParticleSystemData::ShapeModule& module)
{
    if (!ImGui::CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetShapeSchema());
}

bool ParticleParameterPanel::DrawSizeOverLifetime(ParticleSystemData::SizeOverLifetimeModule& module)
{
    if (!ImGui::CollapsingHeader("Size Over Lifetime")) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetSizeOverLifetimeSchema());
}

bool ParticleParameterPanel::DrawTextureSheetAnimation(ParticleSystemData::TextureSheetAnimation& module)
{
    if (!ImGui::CollapsingHeader("Texture Sheet Animation")) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetTextureSheetAnimationSchema());
}

bool ParticleParameterPanel::DrawRenderer(ParticleSystemData::RendererModule& module)
{
    if (!ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) return false;
    return FieldEditor::DrawFields(
        module,
        ParticleSystemSchema::GetRendererSchema());
}
#pragma endregion
