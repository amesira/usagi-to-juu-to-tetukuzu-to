#include "one_shot_particle_behavior.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/particle_system_component.h"
#include "External/ImGui/imgui.h"
#include <algorithm>

void OneShotParticleBehavior::Start()
{
    m_particles = GetOwner() ? GetOwner()->GetComponent<ParticleSystemComponent>() : nullptr;
    if (m_particles) m_particles->Play();
}

void OneShotParticleBehavior::Update()
{
    if (!m_particles || !GetOwner()) return;
    const bool emissionFinished = !m_particles->Main().loop
        && m_particles->Main().duration > 0.0f
        && m_particles->GetTime() >= m_particles->Main().duration;
    if (!emissionFinished) return;
    const auto& particles = m_particles->Particles();
    const bool hasAliveParticle = std::any_of(particles.begin(), particles.end(),
        [](const ParticleSystemComponent::ParticleData& particle) { return particle.alive; });
    if (!hasAliveParticle) GetOwner()->Destroy();
}

void OneShotParticleBehavior::DrawComponentInspector()
{
    ImGui::TextUnformatted("Destroys this object after the one-shot particle finishes.");
}
