//---------------------------------------------------
// File  ：Engine/Editor/editor_manager.h
// Date  ：2026/07/21
// Author：Miu Kitamura
// 
// ・エディターの管理クラス
// ・Engineから呼び出され、エディターの初期化、描画、終了処理を行う
//---------------------------------------------------
#pragma once

#include "editor_imgui_backend.h"
#include "editor_context.h"
#include "editor_window_manager.h"

#include "BaseEditor/hierarchy_view_window.h"
#include "BaseEditor/inspector_view_window.h"
#include "BaseEditor/scene_view_window.h"
#include "BaseEditor/tool_bar_window.h"
#include "BaseEditor/debug_view_window.h"
#include "BaseEditor/settings_view_window.h"
#include "BaseEditor/game_view_window.h"
#include "BaseEditor/canvas_view_window.h"
#include "ParticleEditor/particle_system_editor_window.h"

class EditorManager {
private:
    // エディターのコンテキスト情報
    EditorContext       m_editorContext;

    // === バックエンドとウィンドウ管理 ===
    EditorImGuiBackend  m_imguiBackend;
    EditorWindowManager m_windowManager;

    // === 各ウィンドウ ===
    HierarchyViewWindow m_hierarchyViewWindow;  // Hierarchyウィンドウ
    InspectorViewWindow m_inspectorViewWindow;  // Inspectorウィンドウ

    ToolBarWindow       m_toolBarWindow;        // ツールバーウィンドウ
    DebugViewWindow     m_debugViewWindow;      // デバッグウィンドウ
    SettingsViewWindow  m_settingsViewWindow;   // 設定ウィンドウ

    SceneViewWindow     m_sceneViewWindow;      // SceneViewウィンドウ
    GameViewWindow      m_gameViewWindow;       // GameViewウィンドウ
    CanvasViewWindow    m_canvasViewWindow;     // CanvasViewウィンドウ
    ParticleSystemEditorWindow m_particleSystemEditorWindow;

public:
    EditorManager()
        : m_hierarchyViewWindow(&m_editorContext)
        , m_inspectorViewWindow(&m_editorContext)
        , m_sceneViewWindow(&m_editorContext)
        , m_toolBarWindow(&m_editorContext)
        , m_debugViewWindow(&m_editorContext)
        , m_settingsViewWindow(&m_editorContext)
        , m_gameViewWindow(&m_editorContext)
        , m_canvasViewWindow(&m_editorContext)
        , m_particleSystemEditorWindow(&m_editorContext)
    {
    }

    /// @brief エディターを初期化する
    void Initialize(HWND hWnd);
    /// @brief エディターを終了する
    void Finalize();
    /// @brief エディターを描画する
    void Render();

    /// @brief エディターのコンテキスト情報を取得する
    EditorContext& GetEditorContext() { return m_editorContext; }

    /// @brief シーン破棄時に呼び出す
    void OnSceneDestroyed();

private:
    void RegisterWindows();
    void DrawMainView();
    void DrawToolbar();
    void SyncWindowLifecycle();

};
