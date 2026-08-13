//---------------------------------------------------
// File  ：_/Editor/ParticleEditor/particle_editor_document.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・ParticleEditorのドキュメント管理クラス（アセットの読み書き、編集状態の管理など）
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <string>

#include "Engine/Asset/ParticleAsset/particle_system_asset.h"
#include "Engine/Asset/ParticleAsset/particle_system_asset_loader.h"

class ParticleEditorDocument {
private:
    // === 編集中のパーティクルアセットとそのパス ===
    ParticleSystemAsset m_asset;
    std::filesystem::path m_assetPath;

    // === 編集状態の管理 ===
    bool m_dirty = false;
    std::string m_statusMessage;

public:
    ParticleEditorDocument() { New(); }

    /// @brief 新しいパーティクルアセットを作成する
    void New();
    /// @brief パーティクルアセットを指定されたパスから開く
    bool Open(const std::filesystem::path& path);
    /// @brief パーティクルアセットを現在のパスに保存する。パスが空の場合は失敗する
    bool Save();
    /// @brief パーティクルアセットを指定されたパスに保存する
    bool SaveAs(std::filesystem::path& path);

    /// @brief 編集中のパーティクルアセットのDescを取得する
    ParticleSystemDesc& GetEditingDesc() { return const_cast<ParticleSystemDesc&>(m_asset.GetDesc()); }
    /// @brief 編集中のパーティクルアセットのDescを取得する（const版）
    const ParticleSystemDesc& GetEditingDesc() const { return m_asset.GetDesc(); }

    /// @brief 編集中のパーティクルアセットのパスを取得する
    const std::filesystem::path& GetAssetPath() const { return m_assetPath; }
    /// @brief 編集中のパーティクルアセットのステータスメッセージを取得する
    const std::string& GetStatusMessage() const { return m_statusMessage; }
    /// @brief 編集中のパーティクルアセットのパスが空でないかどうかを取得する
    bool HasAssetPath() const { return !m_assetPath.empty(); }

    /// @brief 編集中のパーティクルアセットが未保存の変更を持っているかどうかを取得する
    bool IsDirty() const { return m_dirty; }
    /// @brief 編集中のパーティクルアセットが未保存の変更を持っていることをマークする
    void MarkDirty() { m_dirty = true; }

};
