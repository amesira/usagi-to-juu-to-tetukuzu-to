//---------------------------------------------------
// File  ：_/Editor/DataEditor/data_editor_document.h
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataEditorのドキュメント管理クラス（アセットの読み書き、編集状態の管理など）
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <string>

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_loader.h"

class DataEditorDocument {
private:
    // === 編集中のデータアセットとそのパス ===
    std::unique_ptr<DataAsset> m_ptrAsset;
    std::filesystem::path m_assetPath;

    std::string m_assetTypeName; // 編集中のデータアセットの型名

    // === 編集状態の管理 ===
    bool m_dirty = false;
    std::string m_statusMessage;

public:
    DataEditorDocument() = default;

    void New();
    bool Open(const std::filesystem::path& path);
    bool Save();
    bool SaveAs(std::filesystem::path& path);

    DataAsset* GetEditingAsset() { return m_ptrAsset.get(); }
    const DataAsset* GetEditingAsset() const { return m_ptrAsset.get(); }

    void SetEditingAssetTypeName(const std::string& typeName) { m_assetTypeName = typeName; }
    const std::string GetEditingAssetTypeName() const { return m_assetTypeName; }

    /// @brief 編集中のデータアセットのパスを取得
    const std::filesystem::path& GetAssetPath() const { return m_assetPath; }
    /// @brief 編集中のデータアセットのステータスメッセージを取得
    const std::string& GetStatusMessage() const { return m_statusMessage; }
    /// @brief 編集中のデータアセットのパスが空でないかどうかを取得
    bool HasAssetPath() const { return !m_assetPath.empty(); }

    /// @brief 未保存の変更があるかどうかを取得
    bool IsDirty() const { return m_dirty; }
    /// @brief 未保存の変更があることをマークする
    void MarkDirty() { m_dirty = true; }

};