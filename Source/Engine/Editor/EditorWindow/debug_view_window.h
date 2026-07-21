//---------------------------------------------------
// debug_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef DEBUG_VIEW_WINDOW_H
#define DEBUG_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class DebugViewWindow : public IEditorWindow {
public:
    DebugViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;
};

#endif // DEBUG_VIEW_WINDOW_H