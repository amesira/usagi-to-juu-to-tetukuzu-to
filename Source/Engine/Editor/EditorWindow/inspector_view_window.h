//---------------------------------------------------
// inspector_view_window.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/12
//---------------------------------------------------
#ifndef INSPECTOR_VIEW_WINDOW_H
#define INSPECTOR_VIEW_WINDOW_H
#include "Engine/Editor/i_editor_window.h"

class GameObject;
class Component;

class InspectorViewWindow : public IEditorWindow {
private:
    // 大きさ保持用の変数
    ImVec2 m_componentGroupSize = ImVec2(0, 0);
    ImVec2 m_behaviorGroupSize = ImVec2(0, 0);

public:
    InspectorViewWindow(EditorContext* editorContext) : IEditorWindow(editorContext) {}
    void Draw() override;

    // Component表示初め
    static bool BeginComponentSection(Component* comp, const char* name, bool useEnableSetting = true);
    static void EndComponentSection();

private:
    void DrawComponentInspector(GameObject* gameObject);

};

#endif // INSPECTOR_VIEW_WINDOW_H