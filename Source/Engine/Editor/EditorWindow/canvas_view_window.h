//---------------------------------------------------
// canvas_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef CANVAS_VIEW_WINDOW_H
#define CANVAS_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class CanvasViewWindow : public IEditorWindow {
public:
    CanvasViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;
};

#endif // CANVAS_VIEW_WINDOW_H