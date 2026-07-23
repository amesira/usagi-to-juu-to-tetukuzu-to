//---------------------------------------------------
// hierarchy_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef HIERARCHY_VIEW_WINDOW_H
#define HIERARCHY_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class HierarchyViewWindow : public IEditorWindow {
public:
    HierarchyViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;
};

#endif // HIERARCHY_VIEW_WINDOW_H