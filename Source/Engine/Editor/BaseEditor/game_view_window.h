//---------------------------------------------------
// game_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef GAME_VIEW_WINDOW_H
#define GAME_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class GameViewWindow : public IEditorWindow {
public:
    GameViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;
};

#endif // GAME_VIEW_WINDOW_H