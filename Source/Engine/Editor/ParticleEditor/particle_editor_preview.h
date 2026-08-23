//---------------------------------------------------
// File  ：Engine/Editor/ParticleEditor/particle_editor_preview.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのプレビューを管理するクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_asset.h"
#include "Engine/Processor/particle_system_processor.h"

struct EditorContext;
class GameObject;
class ParticleSystemComponent;

class ParticleEditorPreview {
private:
    EditorContext* m_editorContext = nullptr;
    IScene* m_scene = nullptr;

    // === プレビュー用のオブジェクト ===
    ParticleSystemComponent* m_particleSystem = nullptr;

    // MEMO: 通常はPlay中にGameWorldにて更新するが、Edit中はEditorが更新するため、Processorを保持しておく
    ParticleSystemProcessor m_processor;

public:
    explicit ParticleEditorPreview(EditorContext* editorContext);
    ~ParticleEditorPreview();

    /// @brief プレビュー用のパーティクルシステムを作成する
    bool GeneratePreviewObject(const ParticleSystemDesc& desc);

    /// @brief プレビュー用のパーティクルシステムに設定を適用する
    void Apply(const ParticleSystemDesc& desc, bool restart);
    void Update();
    void Cleanup();

    /// @brief シーンリロード時の処理
    void OnSceneDestroyed();

    void Play();
    void Pause();
    void Stop();

    bool IsPlaying() const;

    int GetParticleCount() const;

};
