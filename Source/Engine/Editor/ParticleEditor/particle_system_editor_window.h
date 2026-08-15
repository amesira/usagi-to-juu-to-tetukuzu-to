//---------------------------------------------------
// File  ：Engine/Editor/ParticleEditor/particle_system_editor_window.h
// Date  ：2026/07/21
// Author：Miu Kitamura
// 
// ・ParticleEditor全体を管理するウィンドウクラス
//---------------------------------------------------
#pragma once
#include <array>
#include <filesystem>
#include <vector>

#include "Engine/Editor/i_editor_window.h"
#include "particle_editor_document.h"
#include "particle_editor_preview.h"
#include "particle_parameter_panel.h"

class ParticleSystemEditorWindow : public IEditorWindow {
private:
    // === ParticleEditorを構成するメインモジュール ===
    ParticleEditorDocument  m_document;         // パーティクルアセットのロード・保存・編集を管理するドキュメント
    ParticleEditorPreview   m_preview;          // パーティクルアセットのプレビュー表示を管理するモジュール
    ParticleParameterPanel  m_parameterPanel;   // パーティクルアセットのパラメータ編集を管理するパネル

    // パーティクルアセットのパスを保持するリスト
    std::vector<std::filesystem::path> m_assetPaths;
    // パーティクルアセットのパスを保持するバッファ
    std::array<char, 512> m_pathBuffer = {};
    bool m_liveApplyToScene = true;

    bool m_autoSave = true;
    int m_autoSaveFrameCounter = 0;

public:
    // MEMO: explicitを付けることで、暗黙の型変換を防ぎ、意図しないコンストラクタ呼び出しを避けることができる
    explicit ParticleSystemEditorWindow(EditorContext* editorContext);

    void Draw() override;
    void OnWindowClosed() override;

    /// @brief シーンが破棄されたときの処理
    void OnSceneDestroyed();

private:
    /// @brief ParticleEditorのツールバーを描画する
    void DrawToolbar();
    /// @brief ParticleEditorのアセットリストを描画する
    void DrawAssetList();
    /// @brief ParticleEditorのプレビューを描画する
    void DrawPreview();
    /// @brief ParticleEditorのパラメータ編集パネルを描画する
    void DrawParameters();
    /// @brief ParticleEditorのステータスバーを描画する
    void DrawStatusBar();

    /// @brief パーティクルアセットのリストを更新する
    void RefreshAssetList();
    /// @brief パーティクルアセットのパスバッファを同期する
    void SyncPathBuffer();
    /// @brief 指定されたパスのパーティクルアセットを開く
    /// @param path パーティクルアセットのパス
    bool OpenAsset(const std::filesystem::path& path);
};
