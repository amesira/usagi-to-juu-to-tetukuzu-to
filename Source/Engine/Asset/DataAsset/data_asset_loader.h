//---------------------------------------------------
// File  ：Engine/Asset/DataAsset/data_asset_loader.h
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataAssetのセーブ/ロードをJSON形式で行う
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <type_traits>

#include "data_asset.h"
#include "data_asset_type_id.h"

class DataAssetLoader {
private:
    // DataAssetのキャッシュ
    std::unordered_map<std::filesystem::path, std::unique_ptr<DataAsset>> m_dataAssetCache;

    // タイプごとのDataAssetデフォルトインスタンスを保持するマップ
    std::unordered_map<std::string, std::unique_ptr<DataAsset>> m_defaultAssetMap;

public:
    /// @brief DataAssetLoaderを初期化する
    void Initialize();
    /// @brief DataAssetLoaderを終了する
    void Finalize();

    /// @brief DataAssetをJSON形式で保存する
    bool SaveDataAsset(const std::filesystem::path& filePath, const DataAsset& asset);

    /// @brief DataAssetをJSON形式で読み込む
    bool LoadDataAsset(const std::filesystem::path& filePath, DataAsset& outAsset);

    // ------------ Catch操作を行なう関数↓

    /// @brief 指定された型のDataAssetを取得する。キャッシュの生成もここで行う
    /// @tparam TAsset 指定された型のDataAsset。DataAssetを継承している必要がある
    template<class TAsset>
    TAsset* GetAsset(const std::filesystem::path& filePath, bool createIfMissing)
    {
        static_assert(std::is_base_of<DataAsset, TAsset>::value && "TAsset must be derived from DataAsset");

        DataAsset* baseAsset = nullptr;

        // キャッシュに存在する場合はキャッシュから取得
        const std::filesystem::path cacheKey = filePath.lexically_normal();
        auto it = m_dataAssetCache.find(cacheKey);
        if (it != m_dataAssetCache.end()) {
            baseAsset = it->second.get();
        }

        // キャッシュに無い場合はロードする
        if (baseAsset == nullptr) {
            auto newAsset = std::make_unique<TAsset>();

            if (!LoadDataAsset(cacheKey, *newAsset)) {
                if (!createIfMissing) {
                    return nullptr;
                }

                // ロードに失敗した場合は新規セーブする
                SaveDataAsset(cacheKey, *newAsset);
            }

            baseAsset = newAsset.get();
            m_dataAssetCache[cacheKey] = std::move(newAsset);
        }

        // 型が一致しない場合はnullptrを返す
        if (baseAsset->GetDataAssetTypeID() != DataAssetTypeID::getTypeID<TAsset>()) {
            return nullptr;
        }

        // === デフォルトインスタンスを保持するマップに登録する ===
        m_defaultAssetMap[baseAsset->GetAssetTypeName().data()] = std::make_unique<TAsset>();

        // ダウンキャストして返す
        return static_cast<TAsset*>(baseAsset);
    }

    /// @brief DataAssetをJSON形式で再読み込みする
    /// @return キャッシュにある場合は更新してtrueを返す。キャッシュに無い場合は読み込まない
    bool ReloadDataAsset(const std::filesystem::path& filePath);

    // ----------- デフォルトインスタンス
    /// @brief 指定された型のDataAssetのデフォルトアセットを生成する
    std::unique_ptr<DataAsset> CreateAsset(const std::string& typeName) const
    {
        auto it = m_defaultAssetMap.find(typeName);
        if (it == m_defaultAssetMap.end()) return nullptr;

        const DataAsset* defaultAsset = it->second.get();
        if (!defaultAsset) return nullptr;

        return defaultAsset->Clone();
    }
};
