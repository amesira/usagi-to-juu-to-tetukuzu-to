//===================================================
// File  ：Engine/Editor/ParticleEditor/particle_editor_document.cpp
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのドキュメント管理クラス（アセットの読み書き、編集状態の管理など）
//===================================================
#include "particle_editor_document.h"

#include "Engine/engine_service_locator.h"

namespace {
    #define PARTICLE_LOADER EngineServiceLocator::GetAssetManager()->GetParticleAssetLoader()
}

ParticleEditorDocument::ParticleEditorDocument()
{
    New();
}

/// @brief 新しいパーティクルアセットを作成する
void ParticleEditorDocument::New()
{
    m_asset = ParticleSystemAsset{};
    m_assetPath.clear();
    SetDefaultAsset();
    m_dirty = false;
    m_statusMessage = "New particle asset";
}

/// @brief パーティクルアセットを指定されたパスから開く
bool ParticleEditorDocument::Open(const std::filesystem::path& path)
{
    ParticleSystemAsset loadedAsset;
    const std::string pathString = path.generic_string();

    // パスが空の場合はエラーを返す
    if (pathString.empty())
    {
        m_statusMessage = "Path is empty !!";
        return false;
    }

    // パーティクルアセットをロードする
    bool success = PARTICLE_LOADER->LoadParticle(path, loadedAsset);
    if (!success)
    {
        m_statusMessage = "Failed to load: " + pathString;
        return false;
    }

    // === ロードしたアセットを現在のドキュメントに設定する ===
    m_asset = std::move(loadedAsset);
    m_assetPath = path.lexically_normal();
    m_dirty = false;

    m_statusMessage = "Loaded: " + pathString;
    return true;
}

/// @brief パーティクルアセットを現在のパスに保存する。パスが空の場合は失敗する
/// @return 
bool ParticleEditorDocument::Save()
{
    if (m_assetPath.empty())
    {
        m_statusMessage = "Enter a path and use Save As";
        return false;
    }
    return SaveAs(m_assetPath);
}

/// @brief パーティクルアセットを指定されたパスに保存する
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

    // === アセットのヘッダーを設定する ===
    AssetHeader header = m_asset.GetHeader();
    header.m_type = "ParticleSystem";
    header.m_formatVersion = 1;
    header.m_name = path.stem().string();
    m_asset.SetHeader(header);
    m_asset.SetFilePath(path);

    // === パーティクルアセットを保存する ===
    if (!PARTICLE_LOADER->SaveParticle(path, m_asset))
    {
        m_statusMessage = "Failed to save: " + path.generic_string();
        return false;
    }
    m_assetPath = path.lexically_normal();
    m_dirty = false;

    m_statusMessage = "Saved: " + path.generic_string();
    return true;
}

/// @brief デフォルトのパーティクルアセットを設定する
void ParticleEditorDocument::SetDefaultAsset()
{
    AssetHeader header;
    header.m_type = "ParticleSystem";
    header.m_formatVersion = 1;
    header.m_name = "NewParticleSystem";
    m_asset.SetHeader(header);
    m_asset.GetDesc() = ParticleSystemDesc{};
}
