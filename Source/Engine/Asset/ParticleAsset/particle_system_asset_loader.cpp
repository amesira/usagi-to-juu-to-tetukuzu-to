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

namespace
{
    using json = nlohmann::json;

#pragma region ModuleのJSONシリアライズ/デシリアライズ
    // MainModuleをJSONにシリアライズする
    json SerializeMain(const ParticleSystemData::MainModule& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetMainSchema());
    }

    // MainModuleをJSONからデシリアライズする
    bool DeserializeMain(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.mainModule,
            ParticleSystemSchema::GetMainSchema());
    }

    // EmissionModuleをJSONにシリアライズする
    json SerializeEmission(const ParticleSystemData::EmissionModule& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetEmissionSchema());
    }

    // EmissionModuleをJSONからデシリアライズする
    bool DeserializeEmission(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.emissionModule,
            ParticleSystemSchema::GetEmissionSchema());
    }

    // ShapeModuleをJSONにシリアライズする
    json SerializeShape(const ParticleSystemData::ShapeModule& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetShapeSchema());
    }

    // ShapeModuleをJSONからデシリアライズする
    bool DeserializeShape(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.shapeModule,
            ParticleSystemSchema::GetShapeSchema());
    }

    // SizeOverLifetimeModuleをJSONにシリアライズする
    json SerializeSizeOverLifetime(const ParticleSystemData::SizeOverLifetimeModule& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetSizeOverLifetimeSchema());
    }

    // SizeOverLifetimeModuleをJSONからデシリアライズする
    bool DeserializeSizeOverLifetime(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.sizeOverLifetimeModule,
            ParticleSystemSchema::GetSizeOverLifetimeSchema());
    }

    // TextureSheetAnimationModuleをJSONにシリアライズする
    json SerializeTextureSheetAnimation(const ParticleSystemData::TextureSheetAnimation& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetTextureSheetAnimationSchema());
    }

    // TextureSheetAnimationModuleをJSONからデシリアライズする
    bool DeserializeTextureSheetAnimation(const json& jsonValue, ParticleSystemDesc& desc)
    {
        return FieldSerialization::DeserializeFields(
            jsonValue,
            desc.textureSheetAnimation,
            ParticleSystemSchema::GetTextureSheetAnimationSchema());
    }

    // RendererModuleをJSONにシリアライズする
    json SerializeRenderer(const ParticleSystemData::RendererModule& module)
    {
        return FieldSerialization::SerializeFields(
            module,
            ParticleSystemSchema::GetRendererSchema());
    }

    // RendererModuleをJSONからデシリアライズする
    bool DeserializeRenderer(const json& jsonValue, ParticleSystemDesc& desc)
    {
        auto& module = desc.rendererModule;
        const bool succeeded = FieldSerialization::DeserializeFields(
            jsonValue,
            module,
            ParticleSystemSchema::GetRendererSchema());

        // textureResourceは保存データではないため、参照解決前は空にする。
        module.textureResource = nullptr;
        return succeeded;
    }
#pragma endregion

    /// @brief  JSONから指定のモジュールをデシリアライズする
    /// @param data     元のJSONデータ
    /// @param key      モジュールを示すキー
    /// @param desc     デシリアライズ先のParticleSystemDesc
    /// @param deserialize モジュールのデシリアライズ関数
    /// @return 
    bool DeserializeModule(
        const json& data,
        const char* key,
        ParticleSystemDesc& desc,
        bool (*deserialize)(const json&, ParticleSystemDesc&))
    {
        const auto it = data.find(key);
        if (it == data.end()) return true;
        return it->is_object() && deserialize(*it, desc);
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
    const std::string& filePath,
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
            { "main", SerializeMain(desc.mainModule) },
            { "emission", SerializeEmission(desc.emissionModule) },
            { "shape", SerializeShape(desc.shapeModule) },
            { "sizeOverLifetime", SerializeSizeOverLifetime(desc.sizeOverLifetimeModule) },
            { "textureSheetAnimation", SerializeTextureSheetAnimation(desc.textureSheetAnimation) },
            { "renderer", SerializeRenderer(desc.rendererModule) },
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
    const std::string& filePath,
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
        if (!DeserializeModule(*dataIt, "main", loadedDesc, DeserializeMain) ||
            !DeserializeModule(*dataIt, "emission", loadedDesc, DeserializeEmission) ||
            !DeserializeModule(*dataIt, "shape", loadedDesc, DeserializeShape) ||
            !DeserializeModule(*dataIt, "sizeOverLifetime", loadedDesc, DeserializeSizeOverLifetime) ||
            !DeserializeModule(*dataIt, "textureSheetAnimation", loadedDesc, DeserializeTextureSheetAnimation) ||
            !DeserializeModule(*dataIt, "renderer", loadedDesc, DeserializeRenderer))
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

/// @brief ParticleSystemAssetを再読み込みする。キャッシュにある場合は上書きする
bool ParticleSystemAssetLoader::ReloadParticle(
    const std::string& filePath)
{
    auto it = m_particleAssetCache.find(filePath);
    if (it == m_particleAssetCache.end()) {
        // キャッシュに無い場合は読み込まない
        return false;
    }

    ParticleSystemAsset newAsset;
    if (LoadParticle(filePath, newAsset)) {
        // 成功した場合はキャッシュを更新
        it->second = std::make_unique<ParticleSystemAsset>(std::move(newAsset));
        return true;
    }

    return false;
}

/// @brief ParticleSystemAssetを取得する。キャッシュに無い場合はロードする
ParticleSystemAsset* ParticleSystemAssetLoader::GetParticle(const std::string& filePath)
{
    auto it = m_particleAssetCache.find(filePath);
    if (it != m_particleAssetCache.end()) {
        return it->second.get();
    }

    // キャッシュに無い場合はロードする
    auto newAsset = std::make_unique<ParticleSystemAsset>();
    if (LoadParticle(filePath, *newAsset)) {
        ParticleSystemAsset* assetPtr = newAsset.get();
        m_particleAssetCache[filePath] = std::move(newAsset);
        return assetPtr;
    }

    return nullptr;
}
