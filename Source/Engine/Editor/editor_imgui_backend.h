//---------------------------------------------------
// File  ：Engine/Editor/editor_imgui_backend.h
// Date  ：2025/07/01
// Author：Miu Kitamura
// 
// ・エディターのImGuiバックエンドを管理するクラス
// ・ImGuiの初期化、フレーム開始、フレーム終了などの処理を行う
//---------------------------------------------------
#ifndef MI_IMGUI_MANAGER_H
#define MI_IMGUI_MANAGER_H
#include <windows.h>

class EditorImGuiBackend {
public:
    void Initialize(HWND hwnd);
    void Finalize();
    void BeginFrame();
    void EndFrame();

};

#endif