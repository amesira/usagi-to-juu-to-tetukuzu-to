//===================================================
// File  ：_/Editor/DataEditor/data_editor_document.cpp
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataEditorのドキュメント管理クラス（アセットの読み書き、編集状態の管理など）
//===================================================
#include "data_editor_document.h"

#include <iostream>
#include <filesystem>

#include "Engine/engine_service_locator.h"

namespace {
    #define DATA_LOADER EngineServiceLocator::GetAssetManager()->GetDataAssetLoader()
}

/// @brief 新しいデータアセットを作成する
void DataEditorDocument::New()
{
    m_ptrAsset = DATA_LOADER->CreateAsset(m_assetTypeName);
    m_assetPath.clear();
    m_dirty = false;
    m_statusMessage = "New data asset";

    // === 新しいデータアセットを設定する ===
    if (m_ptrAsset)
    {
        m_ptrAsset->SetHeader(AssetHeader{ 
            .m_type = m_ptrAsset->GetAssetTypeName().data(),
            .m_formatVersion = m_ptrAsset->GetSupportedFormatVersion(),
            .m_name = "NewAsset"
            });
    }
}

/// @brief データアセットを指定されたパスから開く
bool DataEditorDocument::Open(const std::filesystem::path& path, const std::string& assetTypeName)
{
    const std::string pathString = path.generic_string();

    if (pathString.empty())
    {   // パスが空
        m_statusMessage = "Path is empty !!";
        return false;
    }

    // === データアセットをロードする ===
    m_assetTypeName = assetTypeName;
    std::unique_ptr<DataAsset> loadedAsset = DATA_LOADER->CreateAsset(m_assetTypeName);
    if (!loadedAsset)
    {   // 指定された型のデータアセットが存在しない
        m_statusMessage = "Failed to create asset of type: " + m_assetTypeName;
        return false;
    }

    if (!DATA_LOADER->LoadDataAsset(path, *loadedAsset))
    {   // ロードに失敗
        m_statusMessage = "Failed to load: " + pathString;
        return false;
    }

    // === ロードしたアセットを現在のドキュメントに設定する ===
    m_ptrAsset = std::move(loadedAsset);
    m_assetPath = path.lexically_normal();
    m_dirty = false;

    m_statusMessage = "Loaded: " + pathString;
    return true;
}

/// @brief データアセットを現在のパスに保存する。パスが空の場合は失敗する
bool DataEditorDocument::Save()
{
    if (m_assetPath.empty())
    {   // パスが空
        m_statusMessage = "Enter a path and use Save As";
        return false;
    }

    return SaveAs(m_assetPath);
}

/// @brief データアセットを指定されたパスに保存する
bool DataEditorDocument::SaveAs(std::filesystem::path& path)
{
    if (path.empty())
    {   // パスが空
        m_statusMessage = "Asset path is empty";
        return false;
    }

    // ディレクトリが存在しない場合は作成する
    std::error_code error;
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error)
        {   // ディレクトリ作成に失敗
            m_statusMessage = "Failed to create asset directory";
            return false;
        }
    }

    if (path.extension() != ".json" || path.stem().extension() != ".data")
    {   // 拡張子が.data.jsonではない
        path.replace_extension(".data.json");
    }

    // === データアセットを保存する ===
    AssetHeader header = m_ptrAsset->GetHeader();
    header.m_name = path.stem().string();
    m_ptrAsset->SetHeader(header);
    m_ptrAsset->SetFilePath(path);

    if (!DATA_LOADER->SaveDataAsset(path, *m_ptrAsset))
    {   // 保存に失敗
        m_statusMessage = "Failed to save: " + path.generic_string();
        return false;
    }
    m_assetPath = path.lexically_normal();
    m_dirty = false;

    DATA_LOADER->ReloadDataAsset(path); // キャッシュを更新する

    m_statusMessage = "Saved: " + path.generic_string();
    return true;
}