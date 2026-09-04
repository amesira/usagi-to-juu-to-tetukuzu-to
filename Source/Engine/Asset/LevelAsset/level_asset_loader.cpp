//===================================================
// File  ：_/Asset/LevelAsset/level_asset_loader.cpp
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "level_asset_loader.h"
#include "level_schema.h"
#include "Engine/Asset/Schema/field_serializer.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set>

using json = nlohmann::json;

void LevelAssetLoader::Initialize() { m_cache.clear(); }
void LevelAssetLoader::Finalize() { m_cache.clear(); }

bool LevelAssetLoader::Save(
    const std::filesystem::path& filePath,
    const LevelAsset& asset) const
{
    const AssetHeader& header = asset.GetHeader();
    const json root = {
        { "type", "Level" },
        { "formatVersion", SupportedVersion },
        { "name", header.m_name },
        { "data", FieldSerialization::SerializeFields(
            asset.GetData(), LevelSchema::GetSchema()) },
    };

    std::ofstream stream(filePath);
    if (!stream.is_open()) return false;
    try
    {
        stream << root.dump(4) << '\n';
        return stream.good();
    }
    catch (const json::exception&)
    {
        return false;
    }
}

bool LevelAssetLoader::Load(
    const std::filesystem::path& filePath,
    LevelAsset& outAsset) const
{
    std::ifstream stream(filePath);
    if (!stream.is_open()) return false;

    try
    {
        json root;
        stream >> root;
        if (!root.is_object() || root.value("type", "") != "Level" ||
            root.value("formatVersion", 0) != SupportedVersion) return false;

        const auto data = root.find("data");
        if (data == root.end() || !data->is_object()) return false;

        LevelData loaded;
        if (!FieldSerialization::DeserializeFields(
            *data, loaded, LevelSchema::GetSchema())) return false;

        std::unordered_set<std::string> objectIds;
        for (const LevelObjectData& object : loaded.objects)
        {
            if (object.id.empty() || !objectIds.insert(object.id).second) return false;
            if (object.collider.type == LevelColliderType::Sphere &&
                object.collider.sphereRadius < 0.0f) return false;
        }

        AssetHeader header;
        header.m_type = "Level";
        header.m_formatVersion = SupportedVersion;
        header.m_name = root.value("name", "Unnamed Level");

        outAsset.SetHeader(header);
        outAsset.SetFilePath(filePath);
        outAsset.GetData() = std::move(loaded);
        return true;
    }
    catch (const json::exception&)
    {
        return false;
    }
}

LevelAsset* LevelAssetLoader::Get(const std::filesystem::path& filePath)
{
    const auto key = filePath.lexically_normal();
    const auto found = m_cache.find(key);
    if (found != m_cache.end()) return found->second.get();

    auto asset = std::make_unique<LevelAsset>();
    if (!Load(key, *asset)) return nullptr;

    LevelAsset* result = asset.get();
    m_cache[key] = std::move(asset);
    return result;
}

bool LevelAssetLoader::Reload(const std::filesystem::path& filePath)
{
    const auto key = filePath.lexically_normal();
    const auto found = m_cache.find(key);
    if (found == m_cache.end()) return false;
    return Load(key, *found->second);
}
