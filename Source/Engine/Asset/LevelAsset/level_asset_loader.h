//----------------------------------------------------
// File  ：_/Asset/LevelAsset/level_asset_loader.cpp
// Date  ：2026/09/02
// Author：Miu Kitamura
//----------------------------------------------------
#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include "level_asset.h"

class LevelAssetLoader {
private:
    static constexpr int SupportedVersion = 1;
    std::unordered_map<std::filesystem::path, std::unique_ptr<LevelAsset>> m_cache;

public:
    void Initialize();
    void Finalize();

    bool Save(const std::filesystem::path& filePath, const LevelAsset& asset) const;
    bool Load(const std::filesystem::path& filePath, LevelAsset& outAsset) const;

    LevelAsset* Get(const std::filesystem::path& filePath);
    bool Reload(const std::filesystem::path& filePath);
};
