//---------------------------------------------------
// File  ：Engine/Editor/editor_window_manager.h
// Date  ：2026/07/21
// Author：Miu Kitamura
// 
// ・エディターのウィンドウ管理クラス
// ・EditorWindowIdに基づいてウィンドウを登録、描画、開閉する
//---------------------------------------------------
#pragma once
#include <array>
#include <string>

#include "External/ImGui/imgui.h"

class IEditorWindow;

// ウィンドウの種類を識別するための列挙型
enum class EditorWindowId
{
    Hierarchy,
    Inspector,
    Debug,

    GameView,
    SceneView,
    CanvasView,

    Settings,

    ParticleSystemEditor,
    DataEditor,

    Count,
};

class EditorWindowManager {
private:
    // ウィンドウの情報を保持する構造体
    struct WindowEntry
    {
        IEditorWindow* window = nullptr;
        std::string title;
        std::string internalId;
        ImVec2 defaultPosition = { 0.0f, 0.0f };
        ImVec2 defaultSize = { 0.0f, 0.0f };
        bool open = false;
        bool registered = false;
    };

    // ウィンドウの情報を格納する配列
    std::array<WindowEntry, static_cast<size_t>(EditorWindowId::Count)> m_windows;

public:
    /// @brief ウィンドウを登録する
    /// @param id
    /// @param window
    /// @param title
    /// @param internalId（ImGuiでのユニークIDとして使用）
    /// @param openByDefault
    /// @param defaultPosition
    /// @param defaultSize
    void Register(
        EditorWindowId id,
        IEditorWindow* window,
        const char* title,
        const char* internalId,
        bool openByDefault,
        ImVec2 defaultPosition,
        ImVec2 defaultSize);

    /// @brief 登録されたすべてのウィンドウを描画する
    void DrawAll();

    /// @brief 指定されたウィンドウを開く
    void Open(EditorWindowId id);
    /// @brief 指定されたウィンドウを閉じる
    void Close(EditorWindowId id);
    /// @brief 指定されたウィンドウの開閉状態を切り替える
    void Toggle(EditorWindowId id);

    /// @brief 指定されたウィンドウが開いているかどうかを確認する
    bool IsOpen(EditorWindowId id) const;

private:
    WindowEntry* Find(EditorWindowId id);
    const WindowEntry* Find(EditorWindowId id) const;

};
