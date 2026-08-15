//===================================================
// File  ：Engine/Editor/editor_manager.cpp
// Date  ：2026/07/21
// Author：Miu Kitamura
// 
// ・エディターの管理クラス
// ・Engineから呼び出され、エディターの初期化、描画、終了処理を行う
//===================================================
#include "editor_manager.h"

void EditorManager::Initialize(HWND hWnd)
{
    m_imguiBackend.Initialize(hWnd);

    m_toolBarWindow.SetWindowManager(&m_windowManager);
    RegisterWindows();
}

void EditorManager::Finalize()
{
    m_particleSystemEditorWindow.OnWindowClosed();
    m_imguiBackend.Finalize();
}

// EditorManagerの描画処理
void EditorManager::Render()
{
    m_imguiBackend.BeginFrame();

    // Main Viewを最初に描画し、各ツールをその上へ重ねる。
    DrawMainView();
    DrawToolbar();
    SyncWindowLifecycle();

    if (m_editorContext.toolbarExpanded) {
        m_windowManager.DrawAll();
    }

    m_imguiBackend.EndFrame();
}

/// @brief シーン破棄時に呼び出す
void EditorManager::OnSceneDestroyed()
{
    m_particleSystemEditorWindow.OnSceneDestroyed();
}

void EditorManager::RegisterWindows()
{
    m_windowManager.Register(
        EditorWindowId::Hierarchy,
        &m_hierarchyViewWindow,
        "Hierarchy",
        "HierarchyView",
        true,
        { 20.0f, 80.0f },
        { 320.0f, 600.0f });

    m_windowManager.Register(
        EditorWindowId::Inspector,
        &m_inspectorViewWindow,
        "Runtime Inspector",
        "RuntimeInspector",
        true,
        { 1560.0f, 80.0f },
        { 340.0f, 860.0f });

    m_windowManager.Register(
        EditorWindowId::Debug,
        &m_debugViewWindow,
        "Debug",
        "DebugView",
        false,
        { 360.0f, 760.0f },
        { 900.0f, 280.0f });

    m_windowManager.Register(
        EditorWindowId::GameView,
        &m_gameViewWindow,
        "Game View",
        "FloatingGameView",
        false,
        { 380.0f, 120.0f },
        { 800.0f, 450.0f });

    m_windowManager.Register(
        EditorWindowId::SceneView,
        &m_sceneViewWindow,
        "Scene View",
        "FloatingSceneView",
        false,
        { 420.0f, 150.0f },
        { 800.0f, 450.0f });

    m_windowManager.Register(
        EditorWindowId::CanvasView,
        &m_canvasViewWindow,
        "Canvas View",
        "FloatingCanvasView",
        false,
        { 460.0f, 180.0f },
        { 800.0f, 450.0f });

    m_windowManager.Register(
        EditorWindowId::Settings,
        &m_settingsViewWindow,
        "Settings",
        "SettingsView",
        false,
        { 1200.0f, 100.0f },
        { 340.0f, 650.0f });

    m_windowManager.Register(
        EditorWindowId::ParticleSystemEditor,
        &m_particleSystemEditorWindow,
        "Particle System Editor",
        "ParticleSystemEditor",
        false,
        { 120.0f, 80.0f },
        { 1600.0f, 850.0f });

    m_windowManager.Register(
        EditorWindowId::DataEditor,
        &m_dataEditorWindow,
        "Data Editor",
        "DataEditor",
        false,
        { 120.0f, 80.0f },
        { 1600.0f, 850.0f });
}

void EditorManager::SyncWindowLifecycle()
{
    if (!m_windowManager.IsOpen(EditorWindowId::ParticleSystemEditor))
    {
        m_particleSystemEditorWindow.OnWindowClosed();
    }

    if (!m_windowManager.IsOpen(EditorWindowId::DataEditor))
    {
        m_dataEditorWindow.OnWindowClosed();
    }
}

/// @brief エディターのメインビューを描画する
void EditorManager::DrawMainView()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    if (ImGui::Begin("Main View###MainView", nullptr, flags))
    {
        switch (m_editorContext.mainViewMode)
        {
        case EditorContext::MainViewMode::Game:
            m_gameViewWindow.Draw();
            break;

        case EditorContext::MainViewMode::Scene:
            m_sceneViewWindow.Draw();
            break;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

/// @brief エディターのツールバーを描画する
void EditorManager::DrawToolbar()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float toolbarHeight = m_editorContext.toolbarExpanded ? 48.0f : 30.0f;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize({ viewport->WorkSize.x, toolbarHeight });
    ImGui::SetNextWindowBgAlpha(0.94f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 6.0f, 4.0f });
    if (ImGui::Begin("Toolbar###MainToolbar", nullptr, flags))
    {
        m_toolBarWindow.Draw();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
