#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>
#include "environment_asset.h"

class EnvironmentAssetLoader {
private:
    static constexpr int SupportedVersion = 1;
    std::unordered_map<std::filesystem::path, std::unique_ptr<EnvironmentAsset>> m_cache;

public:
    void Initialize();
    void Finalize();

    bool Save(const std::filesystem::path& filePath, const EnvironmentAsset& asset) const;
    bool Load(const std::filesystem::path& filePath, EnvironmentAsset& outAsset) const;
    EnvironmentAsset* Get(const std::filesystem::path& filePath);
    bool Reload(const std::filesystem::path& filePath);
};
