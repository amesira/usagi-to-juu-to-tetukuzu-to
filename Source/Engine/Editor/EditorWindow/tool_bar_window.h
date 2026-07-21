//---------------------------------------------------
// tool_bar_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef TOOL_BAR_WINDOW_H
#define TOOL_BAR_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class EditorWindowManager;

class ToolBarWindow : public IEditorWindow {
private:
    EditorWindowManager* m_windowManager = nullptr;

public:
    ToolBarWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;

    void SetWindowManager(EditorWindowManager* windowManager)
    {
        m_windowManager = windowManager;
    }
};

#endif // TOOL_BAR_WINDOW_H
