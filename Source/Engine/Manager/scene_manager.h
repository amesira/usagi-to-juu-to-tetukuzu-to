//---------------------------------------------------
// scene_manager.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/09
//---------------------------------------------------
#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H
#include "Engine/Core/scene_transition.h"
#include <functional>

class IScene;
class SceneBase;

class GameWorld;

class SceneManager {
public:
    enum class SceneID {
        None = 0,
        Title,
        Game,
        Result,
        Max
    };

private:
    SceneBase* m_pScene = nullptr;

    SceneID m_currentScene = SceneID::None;
    SceneID m_nextScene = SceneID::None;
    SceneTransition m_transition;
    std::function<void()> m_beforeSceneRelease;

public:
    void Initialize();
    void Finalize();
    void Update();
    void Draw();

    // シーンの切り替え
    void ChangeScene(SceneID sceneId);
    bool ChangeSceneWithFade(SceneID sceneId, float fadeOutDuration = 0.4f, float fadeInDuration = 0.4f);
    const SceneTransition& GetTransition() const { return m_transition; }
    void NotifyFramePresented() { m_transition.NotifyFramePresented(); }
    void SetBeforeSceneRelease(std::function<void()> callback) { m_beforeSceneRelease = callback; }

    // 現在のシーンを取得
    IScene* GetCurrentScene() const;

private:
    // シーンの終了と破棄
    void ReleaseScene();

    // シーンの生成と初期化
    void LoadScene(SceneID sceneId);

public:
    // シーンのリロード
    void ReloadScene() {
        m_transition.Reset();
        m_nextScene = m_currentScene;
        ReleaseScene();
        LoadScene(m_currentScene);
    }
};

#endif // SCENE_MANAGER_H
