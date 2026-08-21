//---------------------------------------------------
// File  ：_/Asset/MeshEffectAsset/mesh_effect_asset_loader.h
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffectのアセットセーブ/ロードをJSON形式で行う
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <functional>
#include <utility>

#include "mesh_effect_asset.h"

class MeshEffectAssetLoader {
private:
    int m_supportedVersion = 1; // サポートするフォーマットバージョン

    // MeshEffectAssetのキャッシュ
    std::unordered_map<std::filesystem::path, std::unique_ptr<MeshEffectAsset>> m_meshEffectAssetCache;

    // MeshEffectAssetの再読み込み時に呼び出されるコールバック関数
    using ReloadCallback = std::function<void(
        const std::filesystem::path&,
        const MeshEffectAsset&)>;
    ReloadCallback m_reloadCallback;

public:
    void Initialize();
    void Finalize();

    // === MeshEffectAssetのセーブ/ロード関数 ===

    bool Save(const std::filesystem::path& filePath, const MeshEffectAsset& asset);
    bool Load(const std::filesystem::path& filePath, MeshEffectAsset& outAsset);

    // ------------ Catch操作を行なう関数↓

    /// @brief MeshEffectAssetを取得する。キャッシュ生成もここで行う
    MeshEffectAsset* Get(const std::filesystem::path& filePath);
    /// @brief MeshEffectAssetをJSON形式で再読み込みする。キャッシュを更新する
    bool Reload(
        const std::filesystem::path& filePath,
        bool notifyScene = true);

    void SetReloadCallback(ReloadCallback callback) {
        m_reloadCallback = std::move(callback);
    }
};
