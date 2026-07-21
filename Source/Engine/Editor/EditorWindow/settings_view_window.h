//---------------------------------------------------
// settings_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/30
//---------------------------------------------------
#ifndef SETTINGS_VIEW_WINDOW_H
#define SETTINGS_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class SettingsViewWindow : public IEditorWindow {
private:

public:
    SettingsViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;

private:
    // シーンカメラの設定UIを描画
    void DrawSceneCameraSettings();
    // ライティングの設定UIを描画
    void DrawLightingSettings();
    // PostProcessingの設定UIを描画
    void DrawPostProcessingSettings();

};

#endif // SETTINGS_VIEW_WINDOW_H