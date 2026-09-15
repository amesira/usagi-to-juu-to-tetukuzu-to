//===================================================
// File  ：_/Asset/MeshEffectAsset/mesh_effect_asset_loader.cpp
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffectのアセットセーブ/ロードをJSON形式で行う
//===================================================
#include "mesh_effect_asset_loader.h"
#include "mesh_effect_schema.h"

#include "Engine/Asset/Schema/enum_field_serializer.h"
#include "Engine/Asset/Schema/field_serializer.h"

#include "Utility/mi_curve_json.h"
#include "Utility/mi_math_json.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
    template<class TObject, class TSchema>
    bool DeserializeModule(
        const json& data,
        const char* key,
        TObject& object,
        const TSchema& schema)
    {
        const auto it = data.find(key);
        if (it == data.end()) return true;
        if (!it->is_object()) return false;

        return FieldSerialization::DeserializeFields(*it, object, schema);
    }
}

void MeshEffectAssetLoader::Initialize()
{
    m_meshEffectAssetCache.clear();
}

void MeshEffectAssetLoader::Finalize()
{
    m_meshEffectAssetCache.clear();
}

bool MeshEffectAssetLoader::Save(const std::filesystem::path& filePath, const MeshEffectAsset& asset)
{
    const MeshEffectDesc& desc = asset.GetDesc();
    const AssetHeader& header = asset.GetHeader();

    json root = {
        { "type", "MeshEffect" },
        { "formatVersion", m_supportedVersion },
        { "name", header.m_name },
        { "data", {
            { "main", FieldSerialization::SerializeFields(desc.mainModule, MeshEffectSchema::GetMainSchema()) },
            { "transform", FieldSerialization::SerializeFields(desc.transformModule, MeshEffectSchema::GetTransformSchema()) },
            { "flipbook", FieldSerialization::SerializeFields(desc.flipbookModule, MeshEffectSchema::GetFlipbookSchema()) },
            { "scroll", FieldSerialization::SerializeFields(desc.scrollModule, MeshEffectSchema::GetScrollSchema()) },
            { "wave", FieldSerialization::SerializeFields(desc.waveModule, MeshEffectSchema::GetWaveSchema()) },
            { "gradient", FieldSerialization::SerializeFields(desc.gradientModule, MeshEffectSchema::GetGradientSchema()) },
            { "fresnel", FieldSerialization::SerializeFields(desc.fresnelModule, MeshEffectSchema::GetFresnelSchema()) },
            { "renderer", FieldSerialization::SerializeFields(desc.rendererModule, MeshEffectSchema::GetRendererSchema()) }
        } }
    };

    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;

    try {
        ofs << root.dump(4) << '\n';
        return ofs.good();
    }
    catch (const json::exception&) {
        return false;
    }
}

bool MeshEffectAssetLoader::Load(const std::filesystem::path& filePath, MeshEffectAsset& outAsset)
{
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    try {
        json root;
        ifs >> root;

        if (!root.is_object() || root.value("type", "") != "MeshEffect") return false;

        const int version = root.value("formatVersion", 0);
        if (version != m_supportedVersion) return false;

        const auto dataIt = root.find("data");
        if (dataIt == root.end() || !dataIt->is_object()) return false;

        MeshEffectDesc loadedDesc{};
        if (!DeserializeModule(*dataIt, "main", loadedDesc.mainModule, MeshEffectSchema::GetMainSchema()) ||
            !DeserializeModule(*dataIt, "transform", loadedDesc.transformModule, MeshEffectSchema::GetTransformSchema()) ||
            !DeserializeModule(*dataIt, "flipbook", loadedDesc.flipbookModule, MeshEffectSchema::GetFlipbookSchema()) ||
            !DeserializeModule(*dataIt, "scroll", loadedDesc.scrollModule, MeshEffectSchema::GetScrollSchema()) ||
            !DeserializeModule(*dataIt, "wave", loadedDesc.waveModule, MeshEffectSchema::GetWaveSchema()) ||
            !DeserializeModule(*dataIt, "gradient", loadedDesc.gradientModule, MeshEffectSchema::GetGradientSchema()) ||
            !DeserializeModule(*dataIt, "fresnel", loadedDesc.fresnelModule, MeshEffectSchema::GetFresnelSchema()) ||
            !DeserializeModule(*dataIt, "renderer", loadedDesc.rendererModule, MeshEffectSchema::GetRendererSchema())) {
            return false;
        }

        AssetHeader loadedHeader;
        loadedHeader.m_type = "MeshEffect";
        loadedHeader.m_formatVersion = version;
        loadedHeader.m_name = root.value("name", "UnnamedMeshEffect");

        outAsset.SetHeader(loadedHeader);
        outAsset.SetFilePath(filePath);
        outAsset.GetDesc() = std::move(loadedDesc);
        return true;
    }
    catch (const json::exception&) {
        return false;
    }
}

MeshEffectAsset* MeshEffectAssetLoader::Get(const std::filesystem::path& filePath)
{
    const std::filesystem::path cacheKey = filePath.lexically_normal();
    const auto it = m_meshEffectAssetCache.find(cacheKey);
    if (it != m_meshEffectAssetCache.end()) return it->second.get();

    auto newAsset = std::make_unique<MeshEffectAsset>();
    if (!Load(cacheKey, *newAsset)) return nullptr;

    MeshEffectAsset* assetPtr = newAsset.get();
    m_meshEffectAssetCache[cacheKey] = std::move(newAsset);
    return assetPtr;
}

bool MeshEffectAssetLoader::Reload(const std::filesystem::path& filePath, bool notifyScene)
{
    const std::filesystem::path cacheKey = filePath.lexically_normal();
    const auto it = m_meshEffectAssetCache.find(cacheKey);
    if (it == m_meshEffectAssetCache.end()) return false;

    MeshEffectAsset* assetPtr = it->second.get();
    if (!Load(cacheKey, *assetPtr)) return false;

    if (notifyScene && m_reloadCallback) {
        m_reloadCallback(cacheKey, *assetPtr);
    }
    return true;
}