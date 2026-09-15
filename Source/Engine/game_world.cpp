//===================================================
// game_world.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/03/09
//===================================================
#include "game_world.h"
#include "Utility/debug_renderer.h"
#include "Engine/engine_service_locator.h"
#include "Engine/Core/scene_interface.h"

// GameWorldの初期化
void GameWorld::Initialize()
{
    // Scene管理の初期化
    m_sceneManager.Initialize();

    // Processor群の初期化
    m_physicsProcessor.Initialize();
    m_animationProcessor.Initialize();
    m_spriteAnimationProcessor.Initialize();
    m_particleSystemProcessor.Initialize();
    m_meshEffectProcessor.Initialize();
    m_behaviorProcessor.Initialize();
    m_cameraProcessor.Initialize();
    m_renderProcessor.Initialize();

    // RenderViewの初期化
    m_gameRenderViews.resize(4); // 最大4つのゲーム用RenderViewを確保
    for (RenderView& view : m_gameRenderViews) {
        view.Initialize(1920, 1080);
    }
    m_sceneRenderView.Initialize(1280, 720);
    m_canvasRenderView.Initialize(1280, 720);
}

// GameWorldの終了処理
void GameWorld::Finalize()
{
    if (ParticleSystemAssetLoader* loader =
        EngineServiceLocator::ParticleLoader()) {
        loader->SetReloadCallback({});
    }
    if (MeshEffectAssetLoader* loader =
        EngineServiceLocator::MeshEffectLoader()) {
        loader->SetReloadCallback({});
    }

    // Processor群の終了処理
    m_physicsProcessor.Finalize();
    m_animationProcessor.Finalize();
    m_spriteAnimationProcessor.Finalize();
    m_particleSystemProcessor.Finalize();
    m_meshEffectProcessor.Finalize();
    m_behaviorProcessor.Finalize();
    m_cameraProcessor.Finalize();
    m_renderProcessor.Finalize();

    // Scene管理の終了処理
    m_sceneManager.Finalize();

    // RenderViewの解放
    m_gameRenderViews.clear();
}

// GameWorldの更新処理
void GameWorld::Update()
{
    // Scene管理の更新
    m_sceneManager.Update();
    IScene* scene = m_sceneManager.GetCurrentScene();

    // Processor群の更新
    m_physicsProcessor.Process(scene);  // 物理演算制御プロセッサー処理
    m_animationProcessor.Process(scene); // アニメーション制御プロセッサー処理
    m_spriteAnimationProcessor.Process(scene); // スプライトアニメーション制御プロセッサー処理
    m_particleSystemProcessor.Process(scene);
    m_meshEffectProcessor.Process(scene);
    m_behaviorProcessor.Process(scene); // Behavior制御プロセッサー処理
}

// GameWorldの描画処理
void GameWorld::Render()
{
    IScene* scene = m_sceneManager.GetCurrentScene();

    // デバッグ描画の収集
    m_physicsProcessor.CollectDebugDraw(scene);

    // RenderViewの無効化
    for (RenderView& view : m_gameRenderViews) {
        view.enabled = false;
    }

    // カメラ設定・描画情報の取得
    m_cameraProcessor.Process(scene);
    m_cameraProcessor.SetRenderViews(m_gameRenderViews);
    m_mainGameRenderViewIndex = 0;

    // シーンカメラの描画情報をRenderViewに反映
    SetSceneRenderView(scene);
    
    // 描画制御プロセッサー処理
    for (int i = 0; i < m_gameRenderViews.size(); i++) {
        if (!m_gameRenderViews[i].enabled) continue;

        RenderView& view = m_gameRenderViews[i];
        //view.enableDebugDraw = true; // デバッグ描画を有効化
        m_renderProcessor.BindRenderView(&view);
        m_renderProcessor.Process(scene);
        m_renderProcessor.DrawBlackFade(m_sceneManager.GetTransition().GetAlpha());
    }

    if (m_sceneRenderView.enabled) {
        m_renderProcessor.BindRenderView(&m_sceneRenderView);
        m_renderProcessor.Process(scene);
    }

    if (m_canvasRenderView.enabled) {
        m_renderProcessor.BindRenderView(&m_canvasRenderView);
        m_renderProcessor.Process(scene);
    }
}

// -------------------------------- private

// シーンカメラの描画情報をRenderViewに反映
void GameWorld::SetSceneRenderView(IScene* scene)
{
    // シーンカメラの描画情報をRenderViewに反映
    const SceneViewCameraState& sceneCamera = m_sceneViewCamera;

    {
        RenderView& sceneView = m_sceneRenderView;
        sceneView.enabled = true;

        sceneView.viewMatrix = sceneCamera.GetViewMatrix();
        sceneView.projectionMatrix = sceneCamera.GetProjectionMatrix();
        sceneView.eyePosition = sceneCamera.position;
        sceneView.aspectRatio = sceneCamera.aspect;

        sceneView.enable3D = true;
        sceneView.enableLighting = true;
        sceneView.enableShadowMap = false;
        sceneView.enablePostEffect = false;
        sceneView.enableUI = false;
        sceneView.enableDebugDraw = true;
    }

    {
        RenderView& canvasView = m_canvasRenderView;
        canvasView.enabled = true;

        canvasView.viewMatrix = {};
        canvasView.projectionMatrix = {};
        canvasView.eyePosition = {};
        canvasView.aspectRatio = sceneCamera.aspect;

        canvasView.enable3D = false;
        canvasView.enableLighting = false;
        canvasView.enableShadowMap = false;
        canvasView.enablePostEffect = false;
        canvasView.enableUI = true;
        canvasView.enableDebugDraw = false;
    }
}
