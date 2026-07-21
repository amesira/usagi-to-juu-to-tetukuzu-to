#pragma once

#include "mi_imgui_manager.h"
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

class EditorManager
{
private:
    MiImguiManager m_imguiManager;
    EditorContext m_editorContext;
    EditorWindowManager m_windowManager;

    HierarchyViewWindow m_hierarchyViewWindow;
    InspectorViewWindow m_inspectorViewWindow;
    SceneViewWindow m_sceneViewWindow;
    ToolBarWindow m_toolBarWindow;
    DebugViewWindow m_debugViewWindow;
    SettingsViewWindow m_settingsViewWindow;
    GameViewWindow m_gameViewWindow;
    CanvasViewWindow m_canvasViewWindow;

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
};
