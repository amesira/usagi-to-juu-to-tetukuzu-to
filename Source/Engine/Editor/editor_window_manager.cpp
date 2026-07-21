//===================================================
// File  ：Engine/Editor/editor_window_manager.cpp
// Date  ：2026/07/21
// Author：Miu Kitamura
// 
// ・エディターのウィンドウ管理クラス
// ・EditorWindowIdに基づいてウィンドウを登録、描画、開閉する
//===================================================
#include "editor_window_manager.h"

#include "i_editor_window.h"

/// @brief ウィンドウを登録する
void EditorWindowManager::Register(
    EditorWindowId id,
    IEditorWindow* window,
    const char* title,
    const char* internalId,
    bool openByDefault,
    ImVec2 defaultPosition,
    ImVec2 defaultSize)
{
    WindowEntry& entry = m_windows[static_cast<int>(id)];
    entry.window = window;
    entry.title = title ? title : "Editor Window";
    entry.internalId = internalId ? internalId : entry.title;
    entry.defaultPosition = defaultPosition;
    entry.defaultSize = defaultSize;
    entry.open = openByDefault;
    entry.registered = true;
}

/// @brief 全ての登録されたウィンドウを描画する
void EditorWindowManager::DrawAll()
{
    for (WindowEntry& entry : m_windows)
    {
        if (!entry.registered || !entry.open || !entry.window)
        {
            continue;
        }

        // ウィンドウの位置とサイズを設定（初回のみ）
        ImGui::SetNextWindowPos(entry.defaultPosition, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(entry.defaultSize, ImGuiCond_FirstUseEver);

        // ウィンドウのタイトルに内部IDを付加してユニークにする
        const std::string windowName = entry.title + "###" + entry.internalId;
        if (ImGui::Begin(windowName.c_str(), &entry.open))
        {
            entry.window->Draw();
        }
        ImGui::End();
    }
}

/// @brief 指定されたウィンドウを開く
void EditorWindowManager::Open(EditorWindowId id)
{
    if (WindowEntry* entry = Find(id))
    {
        entry->open = true;
    }
}

/// @brief 指定されたウィンドウを閉じる
void EditorWindowManager::Close(EditorWindowId id)
{
    if (WindowEntry* entry = Find(id))
    {
        entry->open = false;
    }
}

/// @brief 指定されたウィンドウの開閉状態を切り替える
void EditorWindowManager::Toggle(EditorWindowId id)
{
    if (WindowEntry* entry = Find(id))
    {
        entry->open = !entry->open;
    }
}

/// @brief 指定されたウィンドウが開いているかどうかを確認する
bool EditorWindowManager::IsOpen(EditorWindowId id) const
{
    const WindowEntry* entry = Find(id);
    return entry && entry->open;
}

/// @brief 指定されたウィンドウの情報を取得する
EditorWindowManager::WindowEntry* EditorWindowManager::Find(EditorWindowId id)
{
    const size_t index = static_cast<int>(id);
    if (index >= m_windows.size() || !m_windows[index].registered)
    {
        return nullptr;
    }
    return &m_windows[index];
}

/// @brief 指定されたウィンドウの情報を取得する
const EditorWindowManager::WindowEntry* EditorWindowManager::Find(EditorWindowId id) const
{
    const size_t index = static_cast<int>(id);
    if (index >= m_windows.size() || !m_windows[index].registered)
    {
        return nullptr;
    }
    return &m_windows[index];
}
