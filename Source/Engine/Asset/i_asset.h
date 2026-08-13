//---------------------------------------------------
// i_asset.h
// ・アセットデータのインターフェース定義
// 
// Author：Miu Kitamura
// Date  ：2026/07/20
//---------------------------------------------------
#pragma once
#include <filesystem>
#include <string>

// アセットのヘッダ情報
struct AssetHeader {
public:
    std::string m_type;  // アセットの種類
    int m_formatVersion = 1;  // 読み込み形式のver
    std::string m_name;
};

class IAsset {
protected:
    AssetHeader m_header;
    std::filesystem::path m_filePath;

public:
    virtual ~IAsset() = default;

    // アセットのヘッダ情報を取得
    const AssetHeader& GetHeader() const { return m_header; }
    void SetHeader(const AssetHeader& header) { m_header = header; }

    // アセットのファイルパスを取得
    const std::filesystem::path& GetFilePath() const { return m_filePath; }
    void SetFilePath(const std::filesystem::path& filePath) { m_filePath = filePath; }

};
