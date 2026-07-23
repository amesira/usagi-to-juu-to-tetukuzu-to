#include "particle_editor_document.h"

ParticleEditorDocument::ParticleEditorDocument()
{
    New();
}

void ParticleEditorDocument::New()
{
    m_asset = ParticleSystemAsset{};
    m_assetPath.clear();
    SetDefaultAsset();
    m_dirty = false;
    m_statusMessage = "New particle asset";
}

bool ParticleEditorDocument::Open(const std::filesystem::path& path)
{
    ParticleSystemAsset loadedAsset;
    const std::string pathString = path.generic_string();
    if (pathString.empty() || !m_loader.LoadParticle(pathString, loadedAsset))
    {
        m_statusMessage = "Failed to load: " + pathString;
        return false;
    }

    m_asset = std::move(loadedAsset);
    m_assetPath = path.lexically_normal();
    m_dirty = false;
    m_statusMessage = "Loaded: " + pathString;
    return true;
}

bool ParticleEditorDocument::Save()
{
    if (m_assetPath.empty())
    {
        m_statusMessage = "Enter a path and use Save As";
        return false;
    }
    return SaveAs(m_assetPath);
}

bool ParticleEditorDocument::SaveAs(const std::filesystem::path& path)
{
    if (path.empty())
    {
        m_statusMessage = "Asset path is empty";
        return false;
    }

    std::error_code error;
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error)
        {
            m_statusMessage = "Failed to create asset directory";
            return false;
        }
    }

    AssetHeader header = m_asset.GetHeader();
    header.m_type = "ParticleSystem";
    header.m_formatVersion = 1;
    header.m_name = path.stem().string();
    m_asset.SetHeader(header);
    m_asset.SetFilePath(path.generic_string());

    if (!m_loader.SaveParticle(path.generic_string(), m_asset))
    {
        m_statusMessage = "Failed to save: " + path.generic_string();
        return false;
    }

    m_assetPath = path.lexically_normal();
    m_dirty = false;
    m_statusMessage = "Saved: " + path.generic_string();
    return true;
}

bool ParticleEditorDocument::Reload()
{
    if (m_assetPath.empty())
    {
        m_statusMessage = "No asset to reload";
        return false;
    }
    return Open(m_assetPath);
}

void ParticleEditorDocument::SetDefaultAsset()
{
    AssetHeader header;
    header.m_type = "ParticleSystem";
    header.m_formatVersion = 1;
    header.m_name = "NewParticleSystem";
    m_asset.SetHeader(header);
    m_asset.SetDesc(ParticleSystemDesc{});
}
