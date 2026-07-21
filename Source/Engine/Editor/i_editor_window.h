//---------------------------------------------------
// File  ：Engine/Editor/i_editor_window.h
// Date  ：2025/07/01
// Author：Miu Kitamura
// 
// ・imguiを用いたエディターウィンドウのインターフェースクラス
//---------------------------------------------------
#pragma once
#include "External/ImGui/imgui.h"
#include "External/ImGui/imgui_internal.h"

#include "External/ImGui/imgui_impl_dx11.h"
#include "External/ImGui/imgui_impl_win32.h"

class EditorContext;

class IEditorWindow {
protected:
    EditorContext* m_editorContext;

public:
    IEditorWindow(EditorContext* editorContext) : m_editorContext(editorContext) {}
    virtual ~IEditorWindow() = default;
    virtual void Draw() = 0;
};