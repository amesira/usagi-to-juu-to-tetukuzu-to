//---------------------------------------------------
// particle_system_processor.h
// 
// Author：Miu Kitamura
// Date  ：2026/06/02
//---------------------------------------------------
#ifndef PARTICLE_SYSTEM_PROCESSOR_H
#define PARTICLE_SYSTEM_PROCESSOR_H
#include "Engine/Core/processor.h"
#include "Engine/Device/direct3d.h"
#include <filesystem>
#include <vector>
using namespace DirectX;

class TransformComponent;
class ParticleSystemComponent;
class ParticleSystemAsset;

class ParticleSystemProcessor : public Processor {
private:
    struct PendingAssetReload {
        std::filesystem::path path;
        const ParticleSystemAsset* asset = nullptr;
    };

    std::vector<PendingAssetReload> m_pendingAssetReloads;

public:
    void Initialize() override;
    void Finalize() override;
    void Process(IScene* pScene) override;

    void OnParticleAssetReloaded(
        const std::filesystem::path& path,
        const ParticleSystemAsset& asset);

};

#endif // PARTICLE_SYSTEM_PROCESSOR_H
