//===================================================
// File  ：Engine/Editor/editor_imgui_backend.cpp
// Date  ：2025/07/01
// Author：Miu Kitamura
// 
// ・エディターのImGuiバックエンドを管理するクラス
// ・ImGuiの初期化、フレーム開始、フレーム終了などの処理を行う
//===================================================
#include "editor_imgui_backend.h"

#include "Engine/Device/direct3d.h"
#include "i_editor_window.h"

namespace {
    // エディタ用のImguiスタイル設定
    void SetupEditorImguiStyle()
    {
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        {
            // Windowの余白
            style.WindowPadding = ImVec2(5, 5);
            // フレーム（タブタイトルなど）の余白
            style.FramePadding = ImVec2(5, 5);
            style.FrameRounding = 3.0f;

            style.ItemSpacing = ImVec2(8, 6);
            style.ItemInnerSpacing = ImVec2(6, 4);
            style.WindowRounding = 4.0f;
            style.ChildRounding = 4.0f;
            style.PopupRounding = 4.0f;
            style.ScrollbarRounding = 4.0f;
            style.GrabRounding = 3.0f;
            style.TabRounding = 3.0f;
        }

        ImVec4* colors = style.Colors;
        {
            // 濃紺を背景、明るい紺色を操作部分、クリーム色を文字や装飾に使用する
            const ImVec4 navyDark = ImVec4(0.035f, 0.055f, 0.100f, 1.00f);
            const ImVec4 navy = ImVec4(0.065f, 0.095f, 0.160f, 1.00f);
            const ImVec4 navyLight = ImVec4(0.105f, 0.145f, 0.225f, 1.00f);
            const ImVec4 navyAccentDark = ImVec4(0.075f, 0.155f, 0.300f, 1.00f);
            const ImVec4 navyAccent = ImVec4(0.100f, 0.245f, 0.480f, 1.00f);
            const ImVec4 navyAccentLight = ImVec4(0.150f, 0.350f, 0.650f, 1.00f);
            const ImVec4 cream = ImVec4(0.965f, 0.910f, 0.760f, 1.00f);
            const ImVec4 creamMuted = ImVec4(0.690f, 0.650f, 0.555f, 1.00f);
            const ImVec4 creamDark = ImVec4(0.390f, 0.365f, 0.305f, 1.00f);

            // テキスト
            colors[ImGuiCol_Text] = cream;
            colors[ImGuiCol_TextDisabled] = creamMuted;

            // 背景
            colors[ImGuiCol_WindowBg] = navyDark;
            colors[ImGuiCol_ChildBg] = navyDark;
            colors[ImGuiCol_PopupBg] = navy;
            colors[ImGuiCol_MenuBarBg] = navy;

            // 枠・入力欄
            colors[ImGuiCol_Border] = creamDark;
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg] = navy;
            colors[ImGuiCol_FrameBgHovered] = navyLight;
            colors[ImGuiCol_FrameBgActive] = navyAccentDark;

            // タイトルバー
            colors[ImGuiCol_TitleBg] = navy;
            colors[ImGuiCol_TitleBgActive] = navyAccentDark;
            colors[ImGuiCol_TitleBgCollapsed] = navyDark;

            // ボタン・選択項目
            colors[ImGuiCol_Button] = navyAccentDark;
            colors[ImGuiCol_ButtonHovered] = navyAccent;
            colors[ImGuiCol_ButtonActive] = navyAccentLight;
            colors[ImGuiCol_Header] = navyAccentDark;
            colors[ImGuiCol_HeaderHovered] = navyAccent;
            colors[ImGuiCol_HeaderActive] = navyAccentLight;

            // タブ
            colors[ImGuiCol_Tab] = navy;
            colors[ImGuiCol_TabHovered] = navyAccent;
            colors[ImGuiCol_TabSelected] = navyAccentDark;
            colors[ImGuiCol_TabSelectedOverline] = cream;
            colors[ImGuiCol_TabDimmed] = navyDark;
            colors[ImGuiCol_TabDimmedSelected] = navyLight;

            // 各種アクセント
            colors[ImGuiCol_CheckMark] = cream;
            colors[ImGuiCol_SliderGrab] = navyAccent;
            colors[ImGuiCol_SliderGrabActive] = cream;
            colors[ImGuiCol_ResizeGrip] = navyAccentDark;
            colors[ImGuiCol_ResizeGripHovered] = navyAccent;
            colors[ImGuiCol_ResizeGripActive] = cream;
            colors[ImGuiCol_Separator] = creamDark;
            colors[ImGuiCol_SeparatorHovered] = navyAccent;
            colors[ImGuiCol_SeparatorActive] = cream;
            colors[ImGuiCol_NavCursor] = cream;
        }
    }
}

// Imguiの初期化
void EditorImGuiBackend::Initialize(HWND hwnd)
{
    // 1. ImGuiコンテキストを作成
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 2. IO設定
    ImGuiIO& io = ImGui::GetIO();

    // フォント設定
    constexpr const char* editorFontPath = "asset/Font/Makinas-4-Flat.otf";
    ImFont* editorFont = io.Fonts->AddFontFromFileTTF(
        editorFontPath,
        12.0f,
        nullptr,
        io.Fonts->GetGlyphRangesJapanese());

    // ファイルがまだ存在しない場合はImGui標準フォントを使用する
    if (editorFont != nullptr) {
        io.FontDefault = editorFont;
    }

    // 3. プラットフォーム用初期化（Win32）
    ImGui_ImplWin32_Init(hwnd);

    // 4. DirectX11用初期化
    ImGui_ImplDX11_Init(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

    // 5. スタイル設定
    SetupEditorImguiStyle();

}

// Imguiの終了処理
void EditorImGuiBackend::Finalize()
{
    // ImGuiの終了処理
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

// フレーム開始の処理
void EditorImGuiBackend::BeginFrame()
{
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();

    ImGui::NewFrame();
}

// フレーム終了の処理
void EditorImGuiBackend::EndFrame()
{
    // フレーム終了（描画コマンドの準備）
    ImGui::Render();

    // レンダリング（ImGui描画実行）
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
