//---------------------------------------------------
// File  ：Engine/Asset/particle_system_asset_loader.h
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・ParticleSystemAssetのセーブ/ロードをJSON形式で行う
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <unordered_map>
#include <memory>

#include "particle_system_asset.h"

class ParticleSystemAssetLoader {
private:
    int m_supportedVersion = 1; // サポートするフォーマットバージョン

    // ParticleSystemAssetのキャッシュ
    std::unordered_map<std::filesystem::path, std::unique_ptr<ParticleSystemAsset>> m_particleAssetCache;

public:
    /// @brief ParticleSystemAssetLoaderを初期化する
    void Initialize();
    /// @brief ParticleSystemAssetLoaderを終了する
    void Finalize();

    /// @brief ParticleSystemAssetをJSON形式で保存する
    bool SaveParticle(const std::filesystem::path& filePath, const ParticleSystemAsset& asset);
    /// @brief ParticleSystemAssetをJSON形式で読み込む
    bool LoadParticle(const std::filesystem::path& filePath, ParticleSystemAsset& outAsset);

    // ------------ Catch操作を行なう関数↓

    /// @brief ParticleSystemAssetを取得する。キャッシュ生成もここで行う
    ParticleSystemAsset* GetParticle(const std::filesystem::path& filePath);
    /// @brief ParticleSystemAssetをJSON形式で再読み込みする。キャッシュを更新する
    bool ReloadParticle(const std::filesystem::path& filePath);

};
