//===================================================
// File  ：Engine/Editor/ParticleEditor/particle_editor_preview.cpp
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのプレビューを管理するクラス
//===================================================
#include "particle_editor_preview.h"

#include "Engine/Editor/editor_context.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Framework/Component/particle_system_component.h"
#include "Engine/Framework/Component/transform_component.h"
#include "Engine/Graphics/texture_repository.h"
#include "Engine/engine_service_locator.h"
#include "Utility/mi_string.h"


namespace
{
    constexpr const char* PreviewObjectName = "__ParticleEditorPreview__";

    #define TEXTURE_REPOSITORY EngineServiceLocator::GetTextureRepository()
}

ParticleEditorPreview::ParticleEditorPreview(EditorContext* editorContext)
    : m_editorContext(editorContext)
{
    // m_processor.Initialize();
}

ParticleEditorPreview::~ParticleEditorPreview()
{
    // m_processor.Finalize();
}

/// @brief プレビュー用のパーティクルシステムを作成する
bool ParticleEditorPreview::GeneratePreviewObject(const ParticleSystemDesc& desc)
{
    // EditorContextが有効で、シーンが存在する場合のみプレビューオブジェクトを生成する
    if (!m_editorContext || !m_editorContext->scene) return false;

    if (m_scene != m_editorContext->scene || !IsPreviewObjectValid())
    {
        // 既存のプレビューオブジェクトが存在する場合は破棄する
        Cleanup();
        m_scene = m_editorContext->scene;

        // === プレビュー用のGameObjectを生成する ===
        GameObject* object = m_scene->CreateGameObject();
        if (!object) return false;

        // プレビュー用オブジェクトの設定
        object->SetName(PreviewObjectName);
        object->AddComponent<TransformComponent>();
        m_particleSystem = object->AddComponent<ParticleSystemComponent>();
        if (!m_particleSystem)
        {
            object->Destroy();
            return false;
        }

        m_objectId = object->GetID();
        Apply(desc, true); // 初期設定を適用して再生を開始する
    }
    return true;
}

/// @brief プレビュー用のパーティクルシステムに設定を適用する
void ParticleEditorPreview::Apply(const ParticleSystemDesc& desc, bool restart)
{
    if (!m_particleSystem) return;
    m_particleSystem->GetDesc() = desc;

    // 再生タイミングはEditor側で制御するため、PlayOnAwakeは無効化する
    m_particleSystem->Main().playOnAwake = false;

    // === テクスチャリソースの設定 ===
    auto& renderer = m_particleSystem->Renderer();
    renderer.textureResource = nullptr;
    if (!renderer.texturePath.empty()) {
        if (TEXTURE_REPOSITORY) {
            renderer.textureResource = TEXTURE_REPOSITORY->GetTextureResource(MiString::ToWString(renderer.texturePath));
        }
    }
    m_particleSystem->SetTextureResource(renderer.textureResource);

    // restart要求の処理
    if (restart) Play();
}

/// @brief プレビュー用のパーティクルシステムを更新する
void ParticleEditorPreview::Update()
{
    // プレビュー用のオブジェクトが有効でない場合は、パーティクルシステムを破棄する
    if (!IsPreviewObjectValid())
    {
        m_particleSystem = nullptr;
        return;
    }

    // Pause中、またはパーティクルシステムが再生中でない場合は更新しない
    if (m_paused || !m_particleSystem->IsPlaying()) return;

    // === EditorモードがEditの場合のみ、ParticleSystemProcessorを使用してパーティクルシステムを更新する ===
    // FIX: ここにProcessorの更新を持たせるべきかは、もう少し考えた方が良いかも
    if (m_editorContext->currentEditorMode == EditorContext::EditorMode::Edit)
    {
        m_processor.Process(m_scene);
    }
}

/// @brief プレビュー用のオブジェクトを破棄する
void ParticleEditorPreview::Cleanup()
{
    if (IsPreviewObjectValid())
    {
        if (GameObject* object = m_scene->GetGameObjectByID(m_objectId))
        {
            object->SetActive(false);
            object->Destroy();
        }
    }

    // シーンアドレスなどもリセットする
    m_scene = nullptr;
    m_objectId = static_cast<unsigned int>(-1);
    m_particleSystem = nullptr;
    m_paused = false;
}

/// @brief プレビュー用のパーティクルシステムが再生中かどうかを確認する
bool ParticleEditorPreview::IsPlaying() const
{
    return m_particleSystem && m_particleSystem->IsPlaying() && !m_paused;
}

/// @brief プレビュー用のオブジェクトが有効かどうかを確認する
bool ParticleEditorPreview::IsPreviewObjectValid() const
{
    if (!m_scene || !m_particleSystem || m_objectId == static_cast<unsigned int>(-1)) return false;

    // プレビュー用のオブジェクトが存在し、名前が一致するかどうかを確認する
    GameObject* object = m_scene->GetGameObjectByID(m_objectId);
    return object && object->GetName() == PreviewObjectName &&
        object->GetComponent<ParticleSystemComponent>() == m_particleSystem;
}

/// @brief ParticleEditorのPlay
void ParticleEditorPreview::Play()
{
    if (!m_particleSystem) return;
    m_paused = false;
    m_particleSystem->Play();
}

/// @brief ParticleEditorのPause
void ParticleEditorPreview::Pause()
{
    if (!m_particleSystem) return;
    m_paused = true;
    m_particleSystem->Pause();
}

/// @brief ParticleEditorのStop
void ParticleEditorPreview::Stop()
{
    if (!m_particleSystem) return;
    m_paused = false;
    m_particleSystem->Stop();
}
