//===================================================
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_document.cpp
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのドキュメント管理クラス
// （アセットの読み書き、編集状態の管理など）
//===================================================
#include "mesh_effect_editor_document.h"

#include "Engine/Asset/MeshEffectAsset/mesh_effect_asset_loader.h"
#include "Engine/engine_service_locator.h"

void MeshEffectEditorDocument::New()
{
    m_asset = MeshEffectAsset{};
    m_assetPath.clear();
    m_dirty = false;
    m_statusMessage = "New mesh effect asset";

    m_asset.SetHeader(AssetHeader{
        .m_type = "MeshEffect",
        .m_formatVersion = 1,
        .m_name = "NewMeshEffect"
        });
    m_asset.GetDesc() = MeshEffectDesc{};
}

bool MeshEffectEditorDocument::Open(const std::filesystem::path& path)
{
    const std::string pathString = path.generic_string();
    if (pathString.empty()) {
        m_statusMessage = "Path is empty !!";
        return false;
    }

    MeshEffectAssetLoader* loader = EngineServiceLocator::MeshEffectLoader();
    if (!loader) {
        m_statusMessage = "MeshEffect loader is not available";
        return false;
    }

    MeshEffectAsset loadedAsset;
    if (!loader->Load(path, loadedAsset)) {
        m_statusMessage = "Failed to load: " + pathString;
        return false;
    }

    // 成功した場合は、読み込んだアセットをドキュメントに設定する
    m_asset = std::move(loadedAsset);
    m_assetPath = path.lexically_normal();
    m_dirty = false;
    m_statusMessage = "Loaded: " + pathString;
    return true;
}

bool MeshEffectEditorDocument::Save(bool liveApplyToScene)
{
    if (m_assetPath.empty()) {
        m_statusMessage = "Enter a path and use Save As";
        return false;
    }
    return SaveAs(m_assetPath, liveApplyToScene);
}

bool MeshEffectEditorDocument::SaveAs(
    std::filesystem::path& path,
    bool liveApplyToScene)
{
    if (path.empty()) {
        m_statusMessage = "Asset path is empty";
        return false;
    }

    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            m_statusMessage = "Failed to create asset directory";
            return false;
        }
    }

    if (path.extension() != ".json" || path.stem().extension() != ".mesh_effect") {
        path.replace_extension(".mesh_effect.json");
    }

    MeshEffectAssetLoader* loader = EngineServiceLocator::MeshEffectLoader();
    if (!loader) {
        m_statusMessage = "MeshEffect loader is not available";
        return false;
    }

    AssetHeader header = m_asset.GetHeader();
    header.m_name = path.stem().string();
    m_asset.SetHeader(header);
    m_asset.SetFilePath(path);

    if (!loader->Save(path, m_asset)) {
        m_statusMessage = "Failed to save: " + path.generic_string();
        return false;
    }

    // 成功した場合は、保存したパスをドキュメントに設定する
    m_assetPath = path.lexically_normal();
    m_dirty = false;
    loader->Reload(path, liveApplyToScene);
    m_statusMessage = "Saved: " + path.generic_string();
    return true;
}
