//---------------------------------------------------
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_preview.h
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのプレビューを管理するクラス
// （プレビュー用のオブジェクトの生成、設定の適用、更新、破棄など）
//---------------------------------------------------
#pragma once
#include "Engine/Asset/MeshEffectAsset/mesh_effect_asset.h"
#include "Engine/Processor/mesh_effect_processor.h"

struct EditorContext;

class MeshEffectEditorPreview {
private:
    EditorContext* m_editorContext = nullptr;
    MeshEffectProcessor m_processor;

    // プレビュー用のオブジェクト
    class MeshEffectComponent* m_meshEffect = nullptr;

public:
    explicit MeshEffectEditorPreview(EditorContext* editorContext);
    ~MeshEffectEditorPreview();

    /// @brief プレビュー用のMeshEffectオブジェクトを生成
    bool GeneratePreviewObject(const MeshEffectDesc& desc);

    void Apply(const MeshEffectDesc& desc, bool restart);
    void Update();
    void Cleanup();

    void OnSceneDestroyed();

    // === 再生状態の管理 ===
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const;

};
