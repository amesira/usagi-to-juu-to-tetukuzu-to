//---------------------------------------------------
// File  ：_/Editor/DataEditor/data_editor_window.h
// Date  ：2026/08/13
// Author：Miu Kitamura
// 
// ・DataEditorのウィンドウを表すクラス
//---------------------------------------------------
#pragma once
#include <array>
#include <filesystem>
#include <vector>
#include <unordered_map>

#include "Engine/Editor/i_editor_window.h"

#include "data_editor_document.h"

class DataEditorWindow : public IEditorWindow {
private:
    struct DataAssetPathEntry {
        std::string typeName;
        std::vector<std::filesystem::path> paths;
    };
    std::vector<DataAssetPathEntry> m_assetPaths; // データアセットのパスのリスト
    std::vector<std::string> m_assetTypeNames;    // データアセットの型名のリスト
    std::array<char, 256> m_pathBuffer{};         // アセットのパスを保持するバッファ

    DataEditorDocument m_document; // DataEditorのドキュメント管理クラス

public:
    DataEditorWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    virtual ~DataEditorWindow() = default;

    /// @brief DataEditorのウィンドウを描画する
    void Draw() override;

    /// @brief DataEditorのウィンドウが閉じられたときの処理
    void OnWindowClosed() override;

private:
    /// @brief ツールバーを描画する
    void DrawToolbar();
    /// @brief アセットリストを描画する
    void DrawAssetList();
    /// @brief パラメータ編集パネルを描画する
    void DrawParameters();
    /// @brief ステータスバーを描画する
    void DrawStatusBar();

    /// @brief アセットのリストを更新する
    void RefreshAssetList();
    /// @brief アセットのパスバッファを同期する
    void SyncPathBuffer();

    /// @brief 指定されたパスのアセットを開く
    /// @param path アセットのパス
    bool OpenAsset(const std::filesystem::path& path);

};