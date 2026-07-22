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

#include "EditorWindow/hierarchy_view_window.h"
#include "EditorWindow/inspector_view_window.h"
#include "EditorWindow/scene_view_window.h"
#include "EditorWindow/tool_bar_window.h"
#include "EditorWindow/debug_view_window.h"
#include "EditorWindow/settings_view_window.h"
#include "EditorWindow/game_view_window.h"
#include "EditorWindow/canvas_view_window.h"
#include "EditorWindow/ParticleEditor/particle_system_editor_window.h"

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

    void Initialize(HWND hWnd);
    void Finalize();
    void Render();

    EditorContext& GetEditorContext() { return m_editorContext; }

private:
    void RegisterWindows();
    void DrawMainView();
    void DrawToolbar();
    void SyncWindowLifecycle();

};
