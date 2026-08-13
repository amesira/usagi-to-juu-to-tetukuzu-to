//---------------------------------------------------
// File  ：Engine/Asset/DataAsset/data_asset_loader.h
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataAssetのセーブ/ロードをJSON形式で行う
//---------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <type_traits>

#include "data_asset.h"
#include "data_asset_type_id.h"

class DataAssetLoader {
private:
    // DataAssetのキャッシュ
    std::unordered_map<std::string, std::unique_ptr<DataAsset>> m_dataAssetCache;

public:
    /// @brief DataAssetLoaderを初期化する
    void Initialize();
    /// @brief DataAssetLoaderを終了する
    void Finalize();

    /// @brief DataAssetをJSON形式で保存する
    bool SaveDataAsset(const std::string& filePath, const DataAsset& asset);

    /// @brief DataAssetをJSON形式で読み込む
    bool LoadDataAsset(const std::string& filePath, DataAsset& outAsset);

    // ------------ Catch操作を行なう関数↓

    /// @brief 指定された型のDataAssetを取得する。キャッシュの生成もここで行う
    /// @tparam TAsset 指定された型のDataAsset。DataAssetを継承している必要がある
    template<class TAsset>
    TAsset* GetAsset(const std::string& filePath) 
    {
        static_assert(std::is_base_of<DataAsset, TAsset>::value && "TAsset must be derived from DataAsset");

        DataAsset* baseAsset = nullptr;

        // キャッシュに存在する場合はキャッシュから取得
        auto it = m_dataAssetCache.find(filePath);
        if (it != m_dataAssetCache.end()) {
            baseAsset = it->second.get();
        }

        // キャッシュに無い場合はロードする
        if (baseAsset == nullptr) {
            auto newAsset = std::make_unique<TAsset>();
            if (!LoadDataAsset(filePath, *newAsset)) {
                return nullptr;
            }

            baseAsset = newAsset.get();
            m_dataAssetCache[filePath] = std::move(newAsset);
        }

        // 型が一致しない場合はnullptrを返す
        if (baseAsset->GetDataAssetTypeID() != DataAssetTypeID::getTypeID<TAsset>()) {
            return nullptr;
        }

        // ダウンキャストして返す
        return static_cast<TAsset*>(baseAsset);
    }

    /// @brief DataAssetをJSON形式で再読み込みする
    /// @return キャッシュにある場合は更新してtrueを返す。キャッシュに無い場合は読み込まない
    bool ReloadDataAsset(const std::string& filePath);

};