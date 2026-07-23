#pragma once

#include "Engine/Asset/particle_system_asset.h"
#include "Engine/Framework/Processor/particle_system_processor.h"

struct EditorContext;
class GameObject;
class ParticleSystemComponent;

class ParticleEditorPreview
{
private:
    EditorContext* m_editorContext = nullptr;
    IScene* m_scene = nullptr;
    unsigned int m_objectId = static_cast<unsigned int>(-1);
    ParticleSystemComponent* m_particleSystem = nullptr;
    ParticleSystemProcessor m_processor;
    bool m_paused = false;

public:
    explicit ParticleEditorPreview(EditorContext* editorContext);
    ~ParticleEditorPreview();

    bool EnsureCreated(const ParticleSystemDesc& desc);
    void Apply(const ParticleSystemDesc& desc, bool restart);
    void Update();
    void Cleanup();

    void Play();
    void Pause();
    void Stop();
    void Restart();

    bool IsCreated() const { return m_particleSystem != nullptr; }
    bool IsPlaying() const;
    ParticleSystemComponent* GetParticleSystem() { return m_particleSystem; }

private:
    bool IsPreviewObjectValid() const;
    void ResetSimulation();
};
