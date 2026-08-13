//---------------------------------------------------
// File  ：Engine/Editor/ParticleEditor/particle_editor_preview.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのプレビューを管理するクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_asset.h"
#include "Engine/Framework/Processor/particle_system_processor.h"

struct EditorContext;
class GameObject;
class ParticleSystemComponent;

class ParticleEditorPreview {
private:
    EditorContext* m_editorContext = nullptr;
    IScene* m_scene = nullptr;

    // === プレビュー用のオブジェクト ===
    unsigned int m_objectId = static_cast<unsigned int>(-1);
    ParticleSystemComponent* m_particleSystem = nullptr;

    // MEMO: 通常はPlay中にGameWorldにて更新するが、Edit中はEditorが更新するため、Processorを保持しておく
    ParticleSystemProcessor m_processor;

    bool m_paused = false;

public:
    explicit ParticleEditorPreview(EditorContext* editorContext);
    ~ParticleEditorPreview();

    /// @brief プレビュー用のパーティクルシステムを作成する
    bool GeneratePreviewObject(const ParticleSystemDesc& desc);

    /// @brief プレビュー用のパーティクルシステムに設定を適用する
    void Apply(const ParticleSystemDesc& desc, bool restart);
    /// @brief プレビュー用のパーティクルシステムを更新する
    void Update();
    /// @brief プレビュー用のオブジェクトを破棄する
    void Cleanup();

    /// @brief シーンリロード時の処理
    void ReloadScene();

    void Play();
    void Pause();
    void Stop();

    bool IsCreated() const { return m_particleSystem != nullptr; }
    bool IsPlaying() const;
    ParticleSystemComponent* GetParticleSystem() { return m_particleSystem; }

private:
    bool IsPreviewObjectValid() const;

};
