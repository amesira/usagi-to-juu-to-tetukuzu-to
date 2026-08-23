//===================================================
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_preview.cpp
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのプレビューを管理するクラス
// （プレビュー用のオブジェクトの生成、設定の適用、更新、破棄など）
//===================================================
#include "mesh_effect_editor_preview.h"

explicit MeshEffectEditorPreview::MeshEffectEditorPreview(EditorContext* editorContext)
    : m_editorContext(editorContext)
{
    m_processor.Initialize();
}

MeshEffectEditorPreview::~MeshEffectEditorPreview()
{
    m_processor.Finalize();
}

