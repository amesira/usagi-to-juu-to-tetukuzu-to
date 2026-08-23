//---------------------------------------------------
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_document.h
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditorのドキュメント管理クラス
// （アセットの読み書き、編集状態の管理など）
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <string>

#include "Engine/Asset/MeshEffectAsset/mesh_effect_asset.h"

class MeshEffectEditorDocument {
private:
    MeshEffectAsset m_asset;
    std::filesystem::path m_assetPath;

    // 編集状態の管理
    bool m_dirty = false;
    std::string m_statusMessage;

public:
    MeshEffectEditorDocument() { New(); }

    void New();
    bool Open(const std::filesystem::path& path);
    bool Save(bool liveApplyToScene = true);
    bool SaveAs(
        std::filesystem::path& path,
        bool liveApplyToScene = true);

    MeshEffectDesc& GetEditingDesc() { return const_cast<MeshEffectDesc&>(m_asset.GetDesc()); }
    const MeshEffectDesc& GetEditingDesc() const { return m_asset.GetDesc(); }
    
    const std::filesystem::path& GetAssetPath() const { return m_assetPath; }
    bool HasAssetPath() const { return !m_assetPath.empty(); }
    
    // 変更状態の管理
    bool IsDirty() const { return m_dirty; }
    void MarkDirty() { m_dirty = true; }

    /// @brief 現在のステータスメッセージを取得する
    const std::string& GetStatusMessage() const { return m_statusMessage; }
};