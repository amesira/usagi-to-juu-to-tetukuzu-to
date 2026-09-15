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

    void New();
    bool Open(const std::filesystem::path& path);
    bool Save(bool liveApplyToScene = true);
    bool SaveAs(
        std::filesystem::path& path,
        bool liveApplyToScene = true);

    ParticleSystemDesc& GetEditingDesc() { return const_cast<ParticleSystemDesc&>(m_asset.GetDesc()); }
    const ParticleSystemDesc& GetEditingDesc() const { return m_asset.GetDesc(); }

    const std::filesystem::path& GetAssetPath() const { return m_assetPath; }
    bool HasAssetPath() const { return !m_assetPath.empty(); }

    // 編集状態の管理
    bool IsDirty() const { return m_dirty; }
    void MarkDirty() { m_dirty = true; }

    const std::string& GetStatusMessage() const { return m_statusMessage; }

};
