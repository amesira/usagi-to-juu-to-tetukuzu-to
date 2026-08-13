//---------------------------------------------------
// File  ：Engine/Asset/DataAsset/data_asset.h
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataAssetの基底クラスを定義する
// ・DataAssetはシリアライズ、デシリアライズの責務も持つ
// ・AssetTypeNameとFormatVersionはDataAssetの種類ごとに異なるため、派生クラスで定義する
//---------------------------------------------------
#pragma once
#include "Engine/Asset/i_asset.h"
#include <string_view>
#include <nlohmann/json.hpp>
#include <functional>

class IFieldSchema;

class DataAsset : public IAsset {
private:
    // DataAssetのタイプID。実行時に一意に決まるため、毎回同じとは限らない。型一致判定にのみ使用することを想定
    int m_dataAssetTypeID = -1;

public:
    DataAsset(int dataAssetType) : m_dataAssetTypeID(dataAssetType) {}
    virtual ~DataAsset() = default;

    /// @brief DataAssetのタイプIDを取得する
    const int GetDataAssetTypeID() const { return m_dataAssetTypeID; }

    /// @brief DataAssetを複製する
    virtual std::unique_ptr<DataAsset> Clone() const = 0;

    /// @brief DataAssetのフィールドスキーマを取得する
    virtual const IFieldSchema& GetFieldSchema() = 0;

    // === タイプ、フォーマットバージョン ===

    /// @brief DataAssetの種類を表す文字列を取得する
    virtual std::string_view GetAssetTypeName() const = 0;
    /// @brief DataAssetのサポートするフォーマットバージョンを取得する
    virtual int GetSupportedFormatVersion() const = 0;

    // === シリアライズ、デシリアライズ ===

    /// @brief DataAssetのデータをJSON形式でシリアライズする
    virtual nlohmann::json SerializeData() const = 0;
    /// @brief DataAssetのデータをJSON形式からデシリアライズする
    /// @return デシリアライズに成功した場合は、実データに反映した上でtrueを返す
    virtual bool DeserializeDataToApply(const nlohmann::json& jsonData) = 0;

};