#pragma once

#include <array>
#include <string>

#include "External/ImGui/imgui.h"

class IEditorWindow;

enum class EditorWindowId
{
    Hierarchy,
    Inspector,
    Debug,
    GameView,
    SceneView,
    CanvasView,
    Settings,
    Count,
};

class EditorWindowManager
{
private:
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

    std::array<WindowEntry, static_cast<size_t>(EditorWindowId::Count)> m_windows;

public:
    void Register(
        EditorWindowId id,
        IEditorWindow* window,
        const char* title,
        const char* internalId,
        bool openByDefault,
        ImVec2 defaultPosition,
        ImVec2 defaultSize);

    void DrawAll();

    void Open(EditorWindowId id);
    void Close(EditorWindowId id);
    void Toggle(EditorWindowId id);
    bool IsOpen(EditorWindowId id) const;

private:
    WindowEntry* Find(EditorWindowId id);
    const WindowEntry* Find(EditorWindowId id) const;
};
