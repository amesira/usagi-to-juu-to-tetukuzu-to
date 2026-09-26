//---------------------------------------------------
// File  ：Engine/Asset/DataAsset/data_asset.h
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataAssetの基底クラスを定義する
// ・DataAssetはシリアライズ、デシリアライズ、エディタ描画の責務も持つ
// ・AssetTypeNameとFormatVersionはDataAssetの種類ごとに異なるため、派生クラスで定義する
// 
// ・将来的にはSerializeFields処理などをテンプレート化し、処理の責務をLoaderやEditorに任せた方が良いかも
// （ParticleSystemの形式に寄せるイメージ）
//---------------------------------------------------
#pragma once
#include "Engine/Asset/i_asset.h"

#include <string_view>
#include <nlohmann/json.hpp>
#include <functional>

class DataAsset : public IAsset {
private:
    // DataAssetのタイプID。実行時に一意に決まるため、毎回同じとは限らない。型一致判定にのみ使用することを想定
    int m_dataAssetTypeID = -1;

    std::string_view m_assetTypeName = "";
    int m_supportedFormatVersion = 0;

    // リロード時に更新されるデータのバージョン番号。エディター上での変更を検知するために使用する
    std::uint64_t m_revision = 0;

    /// @brief DataAssetがリロードされたときに呼ばれる。リビジョン番号を更新する
    friend class DataAssetLoader;
    void OnDataReloaded() { m_revision++; }

public:
    DataAsset(int dataAssetType,const std::string_view& assetTypeName, const int supportVersion) 
        : m_dataAssetTypeID(dataAssetType), m_assetTypeName(assetTypeName), m_supportedFormatVersion(supportVersion) 
    {
        SetHeader(AssetHeader{ assetTypeName.data(), supportVersion, "" });
    }
    virtual ~DataAsset() = default;

    const int GetDataAssetTypeID() const { return m_dataAssetTypeID; }
    const std::uint64_t GetRevision() const { return m_revision; }

    /// @brief DataAssetのデフォルトインスタンスを作成する
    virtual std::unique_ptr<DataAsset> CreateDefaultInstance() const = 0;


    // === タイプ、フォーマットバージョン ===
    std::string_view GetAssetTypeName() const { return m_assetTypeName; }
    int GetSupportedFormatVersion() const { return m_supportedFormatVersion; }

    // === シリアライズ、デシリアライズ ===
    virtual nlohmann::json SerializeData() const = 0;
    virtual bool DeserializeDataToApply(const nlohmann::json& jsonData) = 0;

    // === エディター ===
    virtual bool DrawDataOnEditor() { return false; } 

};