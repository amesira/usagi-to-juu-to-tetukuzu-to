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
}

ParticleEditorPreview::ParticleEditorPreview(EditorContext* editorContext)
    : m_editorContext(editorContext)
{
    m_processor.Initialize();
}

ParticleEditorPreview::~ParticleEditorPreview()
{
    // The GameWorld can be destroyed before EditorManager. Scene-owned preview
    // data is released by the scene in that case, so do not dereference it here.
    m_processor.Finalize();
}

bool ParticleEditorPreview::EnsureCreated(const ParticleSystemDesc& desc)
{
    if (!m_editorContext || !m_editorContext->scene) return false;

    if (m_scene != m_editorContext->scene || !IsPreviewObjectValid())
    {
        Cleanup();
        m_scene = m_editorContext->scene;

        GameObject* object = m_scene->CreateGameObject();
        if (!object) return false;

        object->SetName(PreviewObjectName);
        object->AddComponent<TransformComponent>();
        m_particleSystem = object->AddComponent<ParticleSystemComponent>();
        if (!m_particleSystem)
        {
            object->Destroy();
            return false;
        }

        m_objectId = object->GetID();
        Apply(desc, true);
    }
    return true;
}

void ParticleEditorPreview::Apply(const ParticleSystemDesc& desc, bool restart)
{
    if (!m_particleSystem) return;

    m_particleSystem->GetDesc() = desc;
    // Preview transport controls own playback; do not auto-resume after Pause/Stop.
    m_particleSystem->Main().playOnAwake = false;

    auto& renderer = m_particleSystem->Renderer();
    renderer.textureResource = nullptr;
    if (!renderer.texturePath.empty())
    {
        if (TextureRepository* repository = EngineServiceLocator::GetTextureRepository())
        {
            renderer.textureResource = repository->GetTextureResource(
                MiString::ToWString(renderer.texturePath));
        }
    }
    m_particleSystem->SetTextureResource(renderer.textureResource);

    if (restart) Restart();
}

void ParticleEditorPreview::Update()
{
    if (!IsPreviewObjectValid())
    {
        m_particleSystem = nullptr;
        return;
    }
    if (m_paused || !m_particleSystem->IsPlaying()) return;

    // GameWorld does not update processors in Edit mode, so the editor advances
    // the scene particle processor only while editing.
    if (m_editorContext->currentEditorMode == EditorContext::EditorMode::Edit)
    {
        m_processor.Process(m_scene);
    }
}

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
    m_scene = nullptr;
    m_objectId = static_cast<unsigned int>(-1);
    m_particleSystem = nullptr;
    m_paused = false;
}

void ParticleEditorPreview::Play()
{
    if (!m_particleSystem) return;
    m_paused = false;
    m_particleSystem->Play();
}

void ParticleEditorPreview::Pause()
{
    if (!m_particleSystem) return;
    m_paused = true;
    m_particleSystem->Stop();
}

void ParticleEditorPreview::Stop()
{
    if (!m_particleSystem) return;
    m_paused = false;
    m_particleSystem->Stop();
    ResetSimulation();
}

void ParticleEditorPreview::Restart()
{
    if (!m_particleSystem) return;
    ResetSimulation();
    Play();
}

bool ParticleEditorPreview::IsPlaying() const
{
    return m_particleSystem && m_particleSystem->IsPlaying() && !m_paused;
}

bool ParticleEditorPreview::IsPreviewObjectValid() const
{
    if (!m_scene || !m_particleSystem || m_objectId == static_cast<unsigned int>(-1)) return false;
    GameObject* object = m_scene->GetGameObjectByID(m_objectId);
    return object && object->GetName() == PreviewObjectName &&
        object->GetComponent<ParticleSystemComponent>() == m_particleSystem;
}

void ParticleEditorPreview::ResetSimulation()
{
    if (!m_particleSystem) return;
    m_particleSystem->Particles().clear();
    m_particleSystem->SetTime(0.0f);
    m_particleSystem->SetEmitAccumulator(0.0f);
    m_particleSystem->SetDistanceAccumulator(0.0f);
}
