//---------------------------------------------------
// game_world.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/09
//---------------------------------------------------
#ifndef GAME_WORLD_H
#define GAME_WORLD_H
#include "./Manager/scene_manager.h"

#include "Engine/Framework/Processor/physics_processor.h"
#include "Engine/Framework/Processor/behavior_processor.h"
#include "Engine/Framework/Processor/camera_processor.h"
#include "Engine/Framework/Processor/render_processor.h"
#include "Engine/Framework/Processor/animation_processor.h"
#include "Engine/Framework/Processor/sprite_animation_processor.h"
#include "Engine/Framework/Processor/particle_system_processor.h"

#include <vector>
#include "Engine/render_view.h"

class GameWorld {
private:
    // Scene管理
    SceneManager    m_sceneManager;

    // Processor群
    PhysicsProcessor   m_physicsProcessor;
    AnimationProcessor m_animationProcessor;
    SpriteAnimationProcessor m_spriteAnimationProcessor;
    ParticleSystemProcessor m_particleSystemProcessor;
    BehaviorProcessor  m_behaviorProcessor;

    std::vector<RenderView> m_gameRenderViews; // ゲーム用RenderView群
    RenderView m_sceneRenderView; // シーン用RenderView
    RenderView m_canvasRenderView; // UI描画用RenderView

    CameraProcessor    m_cameraProcessor;
    RenderProcessor    m_renderProcessor;

    // RenderViewインデックス
    int m_mainGameRenderViewIndex = 0;  // メインゲーム用RenderViewのインデックス

public:
    void Initialize();
    void Finalize();
    void Update();
    void Render();

    // SceneManagerへのアクセス
    SceneManager& GetSceneManager() { return m_sceneManager; }
    // RenderViewへのアクセス
    std::vector<RenderView>& GetGameRenderViews() { return m_gameRenderViews; }
    RenderView& GetSceneRenderView() { return m_sceneRenderView; }
    RenderView& GetGameRenderView() { return m_gameRenderViews[m_mainGameRenderViewIndex]; }
    RenderView& GetCanvasRenderView() { return m_canvasRenderView; }

private:
    // SceneRenderViewの設定
    void SetSceneRenderView(IScene* scene);

};

#endif // GAME_WORLD_H
