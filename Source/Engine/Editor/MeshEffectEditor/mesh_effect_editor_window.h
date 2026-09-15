//---------------------------------------------------
// File  ：_/Editor/MeshEffectEditor/mesh_effect_editor_window.h
// Date  ：2026/08/23
// Author：Miu Kitamura
// 
// ・MeshEffectEditor全体を管理するウィンドウクラス
//---------------------------------------------------
#pragma once
#include <array>
#include <filesystem>
#include <vector>

#include "Engine/Editor/i_editor_window.h"
#include "mesh_effect_editor_document.h"
#include "mesh_effect_editor_parameter.h"
#include "mesh_effect_editor_preview.h"

class MeshEffectEditorWindow : public IEditorWindow {
private:
    MeshEffectEditorDocument m_document;
    MeshEffectEditorPreview m_preview;
    MeshEffectEditorParameter m_parameterPanel;

    std::vector<std::filesystem::path> m_assetPaths;
    std::array<char, 512> m_pathBuffer = {};

    // ライブ適用の有効/無効を切り替えるフラグ
    bool m_liveApplyToScene = true;

    // 自動保存の有効/無効を切り替えるフラグ
    bool m_autoSave = true;
    int m_autoSaveFrameCounter = 0;

public:
    explicit MeshEffectEditorWindow(EditorContext* editorContext);

    void Draw() override;
    void OnWindowClosed() override;

    void OnSceneDestroyed();

private:
    void DrawToolbar();
    void DrawAssetList();
    void DrawPreview();
    void DrawParameters();
    void DrawStatusBar();

    /// @brief アセットリストの更新
    void RefreshAssetList();
    /// @brief アセットパスバッファの同期
    void SyncPathBuffer();

    bool OpenMeshEffectAsset(const std::filesystem::path& assetPath);

};