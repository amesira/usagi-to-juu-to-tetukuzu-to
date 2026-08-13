//===================================================
// File  ：Engine/Asset/DataAsset/data_asset_loader.cpp
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataAssetのセーブ/ロードをJSON形式で行う
//===================================================
#include "data_asset_loader.h"

#include "Engine/Asset/Schema/enum_field_serializer.h"
#include "Engine/Asset/Schema/field_serializer.h"

#include "Utility/mi_curve_json.h"
#include "Utility/mi_math_json.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/// @brief DataAssetLoaderの初期化
void DataAssetLoader::Initialize()
{
    m_dataAssetCache.clear();
}

/// @brief DataAssetLoaderの終了処理
void DataAssetLoader::Finalize()
{
    m_dataAssetCache.clear();
}

/// @brief DataAssetをJSON形式で保存する
bool DataAssetLoader::SaveDataAsset(const std::string& filePath, const DataAsset& asset)
{
    try
    {
        const AssetHeader& header = asset.GetHeader();

        json root = {
        { "type", asset.GetAssetTypeName()},
        { "formatVersion", asset.GetSupportedFormatVersion()},
        { "name", header.m_name },
        { "data", asset.SerializeData()}
        };

        // === 指定のパスにファイルを保存 ===
        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return false;

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

/// @brief DataAssetをJSON形式で読み込む
bool DataAssetLoader::LoadDataAsset(const std::string& filePath, DataAsset& outAsset)
{
    // === アセットファイルを開く ===
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    try
    {
        json root;
        ifs >> root;

        if (!root.is_object() || root.value("type", "") != static_cast<std::string>(outAsset.GetAssetTypeName())) return false;

        // FormatVersionを確認
        const int version = root.value("formatVersion", 0);
        if (version != outAsset.GetSupportedFormatVersion()) return false;

        // Dataを確認
        const auto dataIt = root.find("data");
        if (dataIt == root.end() || !dataIt->is_object()) return false;

        // JSONからDataAssetをデシリアライズし反映する
        if (!outAsset.DeserializeDataToApply(*dataIt)) {
            return false;
        }

        // ヘッダーを設定
        AssetHeader loadedHeader;
        loadedHeader.m_type = static_cast<std::string>(outAsset.GetAssetTypeName());
        loadedHeader.m_formatVersion = version;
        loadedHeader.m_name = root.value("name", "Unnamed" + static_cast<std::string>(outAsset.GetAssetTypeName()));

        // === 全項目の読み込み成功後に反映（実データはデシリアライズ時点で反映する） ===
        outAsset.SetHeader(loadedHeader);
        outAsset.SetFilePath(filePath);
        return true;
    }
    catch (const json::exception& error)
    {
        // JSONの解析に失敗
        return false;
    }
}

// ------------------------------ Catch操作を行なう関数

/// @brief DataAssetをJSON形式で再読み込みする
/// @return キャッシュにある場合は更新してtrueを返す。キャッシュに無い場合は読み込まない
bool DataAssetLoader::ReloadDataAsset(const std::string& filePath)
{
    auto it = m_dataAssetCache.find(filePath);
    if (it == m_dataAssetCache.end()) {
        return false;
    }

    DataAsset* asset = it->second.get();
    if (!LoadDataAsset(filePath, *asset)) {
        return false;
    }

    return true;
}
