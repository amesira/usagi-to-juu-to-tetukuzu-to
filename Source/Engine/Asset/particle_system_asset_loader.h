//---------------------------------------------------
// File  ：Engine/Asset/particle_system_asset_loader.h
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・ParticleSystemAssetのセーブ/ロードをJSON形式で行う
//---------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "particle_system_asset.h"

class ParticleSystemAssetLoader {
private:
    int m_supportedVersion = 1; // サポートするフォーマットバージョン

    // ParticleSystemAssetのキャッシュ
    std::unordered_map<std::string, std::unique_ptr<ParticleSystemAsset>> m_particleAssetCache;

public:
    /// @brief ParticleSystemAssetをJSON形式で保存する
    bool SaveParticle(const std::string& filePath, const ParticleSystemAsset& asset);

    /// @brief ParticleSystemAssetをJSON形式で読み込む
    bool LoadParticle(const std::string& filePath, ParticleSystemAsset& outAsset);
    /// @brief ParticleSystemAssetをJSON形式で再読み込みする。キャッシュにある場合は上書き
    bool ReloadParticle(const std::string& filePath);

    /// @brief ParticleSystemAssetを取得する。キャッシュに無い場合はロードする
    ParticleSystemAsset* GetParticle(const std::string& filePath);

};
