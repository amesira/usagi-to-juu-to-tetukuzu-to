//---------------------------------------------------
// scene_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/30
//---------------------------------------------------
#ifndef SCENE_VIEW_WINDOW_H
#define SCENE_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class SceneViewWindow : public IEditorWindow {
public:
    SceneViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;
};

#endif // SCENE_VIEW_WINDOW_H