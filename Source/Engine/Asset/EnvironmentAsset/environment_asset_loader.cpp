#include "environment_asset_loader.h"
#include "environment_schema.h"
#include "Engine/Asset/Schema/field_serializer.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void EnvironmentAssetLoader::Initialize()
{
    m_cache.clear();
}

void EnvironmentAssetLoader::Finalize()
{
    m_cache.clear();
}

bool EnvironmentAssetLoader::Save(
    const std::filesystem::path& filePath,
    const EnvironmentAsset& asset) const
{
    const AssetHeader& header = asset.GetHeader();
    const json root = {
        { "type", "Environment" },
        { "formatVersion", SupportedVersion },
        { "name", header.m_name },
        { "data", FieldSerialization::SerializeFields(
            asset.GetData(), EnvironmentSchema::GetSchema()) }
    };

    std::ofstream stream(filePath);
    if (!stream.is_open()) return false;

    try {
        stream << root.dump(4) << '\n';
        return stream.good();
    }
    catch (const json::exception&) {
        return false;
    }
}

bool EnvironmentAssetLoader::Load(
    const std::filesystem::path& filePath,
    EnvironmentAsset& outAsset) const
{
    std::ifstream stream(filePath);
    if (!stream.is_open()) return false;

    try {
        json root;
        stream >> root;
        if (!root.is_object() || root.value("type", "") != "Environment") return false;
        if (root.value("formatVersion", 0) != SupportedVersion) return false;

        const auto dataIt = root.find("data");
        if (dataIt == root.end() || !dataIt->is_object()) return false;

        EnvironmentData loadedData{};
        if (!FieldSerialization::DeserializeFields(
            *dataIt, loadedData, EnvironmentSchema::GetSchema())) return false;

        AssetHeader header;
        header.m_type = "Environment";
        header.m_formatVersion = SupportedVersion;
        header.m_name = root.value("name", "Unnamed Environment");

        outAsset.SetHeader(header);
        outAsset.SetFilePath(filePath);
        outAsset.GetData() = std::move(loadedData);
        return true;
    }
    catch (const json::exception&) {
        return false;
    }
}

EnvironmentAsset* EnvironmentAssetLoader::Get(const std::filesystem::path& filePath)
{
    const auto key = filePath.lexically_normal();
    const auto found = m_cache.find(key);
    if (found != m_cache.end()) return found->second.get();

    auto asset = std::make_unique<EnvironmentAsset>();
    if (!Load(key, *asset)) return nullptr;

    EnvironmentAsset* result = asset.get();
    m_cache[key] = std::move(asset);
    return result;
}

bool EnvironmentAssetLoader::Reload(const std::filesystem::path& filePath)
{
    const auto key = filePath.lexically_normal();
    const auto found = m_cache.find(key);
    if (found == m_cache.end()) return false;
    return Load(key, *found->second);
}
