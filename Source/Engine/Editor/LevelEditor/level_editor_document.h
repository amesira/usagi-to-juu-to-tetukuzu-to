//---------------------------------------------------
// File  ：_/Editor/LevelEditor/level_editor_document.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・レベルエディタのドキュメントクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/LevelAsset/level_asset.h"

#include <filesystem>
#include <string>

class LevelEditorDocument {
private:
    LevelAsset m_asset;
    std::filesystem::path m_assetPath;

    bool m_dirty = false;
    std::string m_statusMessage = "No level asset loaded";
    
    unsigned int m_nextObjectNumber = 1;

public:
    void New();
    bool Load(const std::filesystem::path& path);
    bool Save();
    bool SaveAs(std::filesystem::path path);
    bool Reload();

    LevelAsset& GetAsset() { return m_asset; }
    const LevelAsset& GetAsset() const { return m_asset; }
    const std::filesystem::path& GetAssetPath() const { return m_assetPath; }
    bool HasAssetPath() const { return !m_assetPath.empty(); }
    const std::string& GetStatusMessage() const { return m_statusMessage; }
    bool IsDirty() const { return m_dirty; }
    void MarkDirty() { m_dirty = true; }

    LevelObjectData* FindObject(const std::string& id);
    LevelObjectData& AddObject();
    bool RemoveObject(const std::string& id);
    bool ScaleAllObjects(float factor,
        const DirectX::XMFLOAT3& pivot = { 0.0f, 0.0f, 0.0f });

private:
    /// @brief レベル内で一意なIDを生成
    std::string GenerateObjectId();
};
