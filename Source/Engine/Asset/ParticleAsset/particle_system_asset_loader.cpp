//===================================================
// File  ：Engine/Asset/particle_system_asset_loader.cpp
// Date  ：2026/07/20
// Author：Miu Kitamura
// 
// ・ParticleSystemAssetのセーブ/ロードをJSON形式で行う
//===================================================
#include "particle_system_asset_loader.h"
#include "particle_system_data.h"
#include "particle_system_field_serializer.h"
#include "particle_system_schema.h"

#include "Engine/Asset/Schema/enum_field_serializer.h"
#include "Engine/Asset/Schema/field_serializer.h"

#include "Utility/mi_curve_json.h"
#include "Utility/mi_math_json.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
    // === SerializeModuleのEntry関数 ===
    // モジュールが存在するかを確認し、存在する場合はSerializeFieldsを呼び出す
    template<class TObject, class TSchema>
    bool DeserializeModule(
        const json& data,
        const char* key,
        TObject& object,
        const TSchema& schema)
    {
        const auto it = data.find(key);

        // 古いJSONにモジュールがない場合は初期値を維持
        if (it == data.end())
        {
            return true;
        }

        if (!it->is_object())
        {
            return false;
        }

        return FieldSerialization::DeserializeFields(
            *it,
            object,
            schema);
    }
}

/// @brief ParticleSystemAssetLoaderの初期化
void ParticleSystemAssetLoader::Initialize()
{
    m_particleAssetCache.clear();
}

/// @brief ParticleSystemAssetLoaderの終了処理
void ParticleSystemAssetLoader::Finalize()
{
    m_particleAssetCache.clear();
}

/// @brief パーティクルアセットをセーブする
bool ParticleSystemAssetLoader::SaveParticle(
    const std::filesystem::path& filePath,
    const ParticleSystemAsset& asset)
{
    const ParticleSystemDesc& desc = asset.GetDesc();
    const AssetHeader& header = asset.GetHeader();

    // JSON形式にシリアライズ化
    json root = {
        { "type", "ParticleSystem" },
        { "formatVersion", m_supportedVersion },
        { "name", header.m_name },
        { "data", {
            { "main", FieldSerialization::SerializeFields(desc.mainModule, ParticleSystemSchema::GetMainSchema())},
            { "emission", FieldSerialization::SerializeFields(desc.emissionModule, ParticleSystemSchema::GetEmissionSchema())},
            { "shape", FieldSerialization::SerializeFields(desc.shapeModule, ParticleSystemSchema::GetShapeSchema())},
            { "sizeOverLifetime", FieldSerialization::SerializeFields(desc.sizeOverLifetimeModule, ParticleSystemSchema::GetSizeOverLifetimeSchema())},
            { "textureSheetAnimation", FieldSerialization::SerializeFields(desc.textureSheetAnimation, ParticleSystemSchema::GetTextureSheetAnimationSchema())},
            { "renderer", FieldSerialization::SerializeFields(desc.rendererModule, ParticleSystemSchema::GetRendererSchema())},
        } },
    };

    // === 指定のパスにファイルを保存 ===
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) return false;
    try
    {
        // ParticleAssetファイルを出力する
        ofs << root.dump(4) << '\n';
        return ofs.good();
    }
    catch (const json::exception& error)
    {
        // JSONの解析に失敗
        return false;
    }
}

/// @brief パーティクルアセットをロードする
bool ParticleSystemAssetLoader::LoadParticle(
    const std::filesystem::path& filePath,
    ParticleSystemAsset& outAsset)
{
    // === アセットファイルを開く ===
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    try
    {
        json root;
        ifs >> root;

        // Typeを確認
        if (!root.is_object() || root.value("type", "") != "ParticleSystem") return false;

        // FormatVersionを確認
        const int version = root.value("formatVersion", 0);
        if (version != m_supportedVersion) return false;

        // Dataを確認
        const auto dataIt = root.find("data");
        if (dataIt == root.end() || !dataIt->is_object()) return false;

        // JSONからParticlesystemAssetをデシリアライズ
        ParticleSystemDesc loadedDesc{};
        if (!DeserializeModule(*dataIt, "main", loadedDesc.mainModule, ParticleSystemSchema::GetMainSchema()) ||
            !DeserializeModule(*dataIt, "emission", loadedDesc.emissionModule, ParticleSystemSchema::GetEmissionSchema()) ||
            !DeserializeModule(*dataIt, "shape", loadedDesc.shapeModule, ParticleSystemSchema::GetShapeSchema()) ||
            !DeserializeModule(*dataIt, "sizeOverLifetime", loadedDesc.sizeOverLifetimeModule, ParticleSystemSchema::GetSizeOverLifetimeSchema()) ||
            !DeserializeModule(*dataIt, "textureSheetAnimation", loadedDesc.textureSheetAnimation, ParticleSystemSchema::GetTextureSheetAnimationSchema()) ||
            !DeserializeModule(*dataIt, "renderer", loadedDesc.rendererModule, ParticleSystemSchema::GetRendererSchema()))
        {
            return false;
        }

        // ヘッダーを設定
        AssetHeader loadedHeader;
        loadedHeader.m_type = "ParticleSystem";
        loadedHeader.m_formatVersion = version;
        loadedHeader.m_name = root.value("name", "UnnamedParticleSystem");

        // === 全項目の読み込み成功後に反映 ===
        outAsset.SetHeader(loadedHeader);
        outAsset.SetFilePath(filePath);
        outAsset.GetDesc() = std::move(loadedDesc);

        return true;
    }
    catch (const json::exception& error)
    {
        // JSONの解析に失敗
        return false;
    }
}

// --------------------------------- Catch操作を行なう関数

/// @brief ParticleSystemAssetを取得する。キャッシュ生成もここで行う
ParticleSystemAsset* ParticleSystemAssetLoader::GetParticle(const std::filesystem::path& filePath)
{
    const std::filesystem::path cacheKey = filePath.lexically_normal();
    auto it = m_particleAssetCache.find(cacheKey);
    if (it != m_particleAssetCache.end()) {
        return it->second.get();
    }

    // キャッシュに無い場合はロードする
    auto newAsset = std::make_unique<ParticleSystemAsset>();
    if (LoadParticle(cacheKey, *newAsset)) {
        ParticleSystemAsset* assetPtr = newAsset.get();
        m_particleAssetCache[cacheKey] = std::move(newAsset);
        return assetPtr;
    }

    return nullptr;
}

/// @brief ParticleSystemAssetを再読み込みする。キャッシュを更新する
bool ParticleSystemAssetLoader::ReloadParticle(
    const std::filesystem::path& filePath)
{
    const std::filesystem::path cacheKey = filePath.lexically_normal();
    auto it = m_particleAssetCache.find(cacheKey);
    if (it == m_particleAssetCache.end()) {
        // キャッシュに無い場合は読み込まない
        return false;
    }

    ParticleSystemAsset* assetPtr = it->second.get();
    if (!LoadParticle(cacheKey, *assetPtr)) {
        return false;
    }

    return true;;
}
