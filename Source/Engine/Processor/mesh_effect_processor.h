//---------------------------------------------------
// File  ：_/Processor/mesh_effect_processor.h
// Date  ：2026/08/22
// Author：Miu Kitamura
// 
// ・MeshEffectの更新処理を行うProcessor
//---------------------------------------------------
#pragma once
#include "Engine/Core/processor.h"
#include "Engine/Device/direct3d.h"
using namespace DirectX;

#include <filesystem>
#include <vector>

class MeshEffectAsset;

class MeshEffectProcessor : public Processor {
private:
    struct PendingAssetReload {
        std::filesystem::path path;
        const MeshEffectAsset* asset = nullptr;
    };
    std::vector<PendingAssetReload> m_pendingAssetReloads;

public:
    void Initialize() override;
    void Finalize() override;
    void Process(IScene* pScene) override;

    void OnMeshEffectAssetReloaded(
        const std::filesystem::path& path,
        const MeshEffectAsset& asset);
};