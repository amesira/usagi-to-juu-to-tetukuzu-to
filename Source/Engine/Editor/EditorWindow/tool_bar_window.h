//---------------------------------------------------
// tool_bar_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef TOOL_BAR_WINDOW_H
#define TOOL_BAR_WINDOW_H
#include "imgui_window_interface.h"

class EditorWindowManager;

class ToolBarWindow : public IImguiWindow {
private:
    EditorWindowManager* m_windowManager = nullptr;

public:
    ToolBarWindow(EditorContext* editorContext) : IImguiWindow(editorContext) {}
    void Draw() override;

    void SetWindowManager(EditorWindowManager* windowManager)
    {
        m_windowManager = windowManager;
    }
};

#endif // TOOL_BAR_WINDOW_H
