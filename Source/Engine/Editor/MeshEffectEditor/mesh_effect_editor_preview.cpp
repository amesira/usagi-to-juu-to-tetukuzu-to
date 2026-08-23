//===================================================
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_preview.cpp
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのプレビューを管理するクラス
// （プレビュー用のオブジェクトの生成、設定の適用、更新、破棄など）
//===================================================
#include "mesh_effect_editor_preview.h"

#include "Engine/Editor/editor_context.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/mesh_effect_component.h"
#include "Engine/Component/transform_component.h"

namespace {
    constexpr const char* PreviewObjectName = "__MeshEffectEditorPreview__";
}

MeshEffectEditorPreview::MeshEffectEditorPreview(EditorContext* editorContext)
    : m_editorContext(editorContext)
{
    m_processor.Initialize();
}

MeshEffectEditorPreview::~MeshEffectEditorPreview()
{
    Cleanup();
    m_processor.Finalize();
}

bool MeshEffectEditorPreview::GeneratePreviewObject(const MeshEffectDesc& desc)
{
    if (!m_editorContext || !m_editorContext->scene) return false;
    if (m_meshEffect) return true;

    // プレビュー用のGameObjectを生成
    {
        GameObject* object = m_editorContext->scene->CreateGameObject();
        if (!object) return false;

        object->SetName(PreviewObjectName);
        object->AddComponent<TransformComponent>();
        m_meshEffect = object->AddComponent<MeshEffectComponent>();
        if (!m_meshEffect) {
            object->Destroy();
            return false;
        }
    }

    Apply(desc, true);
    return true;
}

void MeshEffectEditorPreview::Apply(const MeshEffectDesc& desc, bool restart)
{
    if (!m_meshEffect) return;

    m_meshEffect->GetDesc() = desc;
    m_meshEffect->Main().playOnAwake = false;
    m_meshEffect->SetModelResource(nullptr);
    m_meshEffect->SetTextureResource(nullptr);

    if (restart) Play();
}

void MeshEffectEditorPreview::Update()
{
    if (!m_editorContext || !m_editorContext->scene || !m_meshEffect) return;
    if (!m_meshEffect->IsPlaying()) return;

    // EditorモードがEditの場合のみ、MeshEffectProcessorを使用して更新する
    if (m_editorContext->currentEditorMode == EditorContext::EditorMode::Edit) {
        m_processor.Process(m_editorContext->scene);
    }
}

void MeshEffectEditorPreview::Cleanup()
{
    if (m_meshEffect && m_editorContext && m_editorContext->scene) {
        // プレビュー用のGameObjectを破棄する
        if (GameObject* object = m_editorContext->scene->GetGameObjectByID(
            m_meshEffect->GetOwner()->GetID())) {
            object->SetActive(false);
            object->Destroy();
        }
    }
    m_meshEffect = nullptr;
}

void MeshEffectEditorPreview::OnSceneDestroyed()
{
    m_meshEffect = nullptr;
}

#pragma region 再生状態の管理
void MeshEffectEditorPreview::Play()
{
    if (m_meshEffect) m_meshEffect->Play();
}

void MeshEffectEditorPreview::Pause()
{
    if (m_meshEffect) m_meshEffect->Pause();
}

void MeshEffectEditorPreview::Stop()
{
    if (m_meshEffect) m_meshEffect->Stop();
}

bool MeshEffectEditorPreview::IsPlaying() const
{
    if (m_meshEffect == nullptr) return false;
    return m_meshEffect->IsPlaying();
}
#pragma endregion
