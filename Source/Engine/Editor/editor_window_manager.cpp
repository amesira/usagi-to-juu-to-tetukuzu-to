#include "editor_window_manager.h"

#include "EditorWindow/imgui_window_interface.h"

namespace
{
    size_t ToIndex(EditorWindowId id)
    {
        return static_cast<size_t>(id);
    }
}

void EditorWindowManager::Register(
    EditorWindowId id,
    IImguiWindow* window,
    const char* title,
    const char* internalId,
    bool openByDefault,
    ImVec2 defaultPosition,
    ImVec2 defaultSize)
{
    WindowEntry& entry = m_windows[ToIndex(id)];
    entry.window = window;
    entry.title = title ? title : "Editor Window";
    entry.internalId = internalId ? internalId : entry.title;
    entry.defaultPosition = defaultPosition;
    entry.defaultSize = defaultSize;
    entry.open = openByDefault;
    entry.registered = true;
}

void EditorWindowManager::DrawAll()
{
    for (WindowEntry& entry : m_windows)
    {
        if (!entry.registered || !entry.open || !entry.window)
        {
            continue;
        }

        ImGui::SetNextWindowPos(entry.defaultPosition, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(entry.defaultSize, ImGuiCond_FirstUseEver);

        const std::string windowName = entry.title + "###" + entry.internalId;
        if (ImGui::Begin(windowName.c_str(), &entry.open))
        {
            entry.window->Draw();
        }
        ImGui::End();
    }
}

void EditorWindowManager::Open(EditorWindowId id)
{
    if (WindowEntry* entry = Find(id))
    {
        entry->open = true;
    }
}

void EditorWindowManager::Close(EditorWindowId id)
{
    if (WindowEntry* entry = Find(id))
    {
        entry->open = false;
    }
}

void EditorWindowManager::Toggle(EditorWindowId id)
{
    if (WindowEntry* entry = Find(id))
    {
        entry->open = !entry->open;
    }
}

bool EditorWindowManager::IsOpen(EditorWindowId id) const
{
    const WindowEntry* entry = Find(id);
    return entry && entry->open;
}

EditorWindowManager::WindowEntry* EditorWindowManager::Find(EditorWindowId id)
{
    const size_t index = ToIndex(id);
    if (index >= m_windows.size() || !m_windows[index].registered)
    {
        return nullptr;
    }
    return &m_windows[index];
}

const EditorWindowManager::WindowEntry* EditorWindowManager::Find(EditorWindowId id) const
{
    const size_t index = ToIndex(id);
    if (index >= m_windows.size() || !m_windows[index].registered)
    {
        return nullptr;
    }
    return &m_windows[index];
}
