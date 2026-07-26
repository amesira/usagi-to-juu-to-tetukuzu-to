#pragma once

#include <filesystem>
#include <string>

#include "Engine/Asset/ParticleAsset/particle_system_asset.h"
#include "Engine/Asset/ParticleAsset/particle_system_asset_loader.h"

class ParticleEditorDocument
{
private:
    ParticleSystemAssetLoader m_loader;
    ParticleSystemAsset m_asset;
    std::filesystem::path m_assetPath;
    bool m_dirty = false;
    std::string m_statusMessage;

public:
    ParticleEditorDocument();

    void New();
    bool Open(const std::filesystem::path& path);
    bool Save();
    bool SaveAs(const std::filesystem::path& path);
    bool Reload();

    ParticleSystemDesc& GetEditingDesc() { return const_cast<ParticleSystemDesc&>(m_asset.GetDesc()); }
    const ParticleSystemDesc& GetEditingDesc() const { return m_asset.GetDesc(); }

    const std::filesystem::path& GetAssetPath() const { return m_assetPath; }
    const std::string& GetStatusMessage() const { return m_statusMessage; }
    bool HasAssetPath() const { return !m_assetPath.empty(); }
    bool IsDirty() const { return m_dirty; }
    void MarkDirty() { m_dirty = true; }

private:
    void SetDefaultAsset();
};
