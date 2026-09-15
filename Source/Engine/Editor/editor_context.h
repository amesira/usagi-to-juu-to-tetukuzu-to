//---------------------------------------------------
// File  ：Engine/Editor/editor_context.h
// Date  ：2026/05
// Author：Miu Kitamura
// 
// ・エディターのコンテキスト情報を保持する構造体
//---------------------------------------------------
#pragma once
#include <string>
#include <vector>

class IScene;
struct RenderView;
class GameObject;
class EnvironmentAsset;
struct SceneViewCameraState;

struct EditorContext {
    enum class MainViewMode {
        Game,
        Scene,
    } mainViewMode = MainViewMode::Game;

    IScene* scene = nullptr;                  // 観測対象のシーン
    EnvironmentAsset* environmentAsset = nullptr;
    SceneViewCameraState* sceneViewCamera = nullptr;

    RenderView* sceneRenderView = nullptr;   // SceneView表示用
    RenderView* gameRenderView = nullptr;    // GameView表示用
    RenderView* canvasRenderView = nullptr;   // CanvasView表示用（UIなどのオーバーレイ描画に使用）

    GameObject* selectedObject = nullptr;    // 現在選択中のオブジェクト

    // ディスプレイサイズ（SceneViewのサイズ計算などで使用）
    float displayX = 1280.0f; // ディスプレイの幅
    float displayY = 720.0f;  // ディスプレイの高さ

    // 各ウィンドウのサイズ（SceneViewのサイズ計算などで使用）
    float toolbarHeight = 40.0f;
    float hierarchyWidth = 300.0f;
    float inspectorWidth = 300.0f;

    // ツールバーの展開状態（true: 展開中、false: 折りたたみ中）
    bool toolbarExpanded = true;

    std::vector<std::string> logMessages; // ログメッセージのリスト

    enum class EditorMode {
        Edit,
        Play,
    } currentEditorMode = EditorMode::Edit;

    bool triggerSceneReload = false; // シーンのリロード要求フラグ

};
