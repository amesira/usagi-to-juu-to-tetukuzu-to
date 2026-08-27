//---------------------------------------------------
// animation_component.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/06
//---------------------------------------------------
#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H
#include "Engine/Core/component.h"

#include <string>
#include <vector>
#include "Engine/Graphics/model_resource.h"

// アニメーションの状態を表す構造体
struct AnimationState {
    std::string clipName = "None";
    int         clipIndex = -1;
    float   timer = 0.0f;
    float   speed = 1.0f; // 再生速度の倍率
    bool    loop = true;
    bool    finished = false;
};

// アニメーションのトランジションを表す構造体
struct AnimationTransition {
    AnimationState* fromState = nullptr;
    AnimationState* toState = nullptr;

    float duration = 0.5f; // トランジションの長さ（秒）
    float timer = 0.0f;    // トランジションの経過時間
    bool isTransitioning = false; // トランジション中かどうか
};

class AnimationComponent : public Component {
private:
    AnimationState m_currentState; // 現在のアニメーション状態

public:
    static constexpr char CLIP_NONE[] = "None";

    /// @brief アニメーションを再生する
    void PlayAnimation(int clipIndex, float speed = 1.0f, bool loop = true, bool restart = false) {
        if (!restart && m_currentState.clipIndex == clipIndex && !m_currentState.finished) return;
        m_currentState.clipName = "CLIP";
        m_currentState.clipIndex = clipIndex;
        m_currentState.timer = 0.0f;
        m_currentState.speed = speed;
        m_currentState.loop = loop;
        m_currentState.finished = false;
    }

    void SetAnimationState(int clipIndex, float speed) {
        PlayAnimation(clipIndex, speed, true, false);
    }
    AnimationState& GetAnimationState() { return m_currentState; }
    const AnimationState& GetAnimationState() const { return m_currentState; }
    bool IsFinished() const { return m_currentState.finished; }

};

#endif // ANIMATION_COMPONENT_H
