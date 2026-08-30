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
#include <algorithm>
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
    AnimationState sourceState;
    float duration = 0.0f;
    float timer = 0.0f;
    bool active = false;
};

/// @brief 1D BlendTreeを構成するアニメーションクリップ
struct AnimationBlendTree1DNode {
    int clipIndex = -1;
    float threshold = 0.0f;
};

/// @brief 1D BlendTreeの再生状態
struct AnimationBlendTree1DState {
    std::vector<AnimationBlendTree1DNode> nodes;
    float parameter = 0.0f;
    float normalizedTime = 0.0f;
    float speed = 1.0f;
    bool loop = true;
    bool finished = false;
};

class AnimationComponent : public Component {
public:
    enum class PlaybackType {
        SingleClip,
        BlendTree1D,
    };

private:
    AnimationState m_currentState; // 現在のアニメーション状態
    AnimationTransition m_transition;
    AnimationBlendTree1DState m_blendTree1DState;
    PlaybackType m_playbackType = PlaybackType::SingleClip;

public:
    static constexpr char CLIP_NONE[] = "None";

    /// @brief アニメーションを再生する
    void PlayAnimation(
        int clipIndex,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false,
        float transitionTime = 0.0f)
    {
        if (!restart &&
            m_playbackType == PlaybackType::SingleClip &&
            m_currentState.clipIndex == clipIndex &&
            !m_currentState.finished) return;

        AnimationState nextState;
        nextState.clipName = "CLIP";
        nextState.clipIndex = clipIndex;
        nextState.timer = 0.0f;
        nextState.speed = speed;
        nextState.loop = loop;
        nextState.finished = false;

        const bool canTransition =
            transitionTime > 0.0f &&
            m_playbackType == PlaybackType::SingleClip &&
            m_currentState.clipIndex >= 0;

        if (canTransition) {
            // 遷移中の再要求では、現在の遷移先を次の遷移元として扱う
            m_transition.sourceState = m_currentState;
            m_transition.duration = transitionTime;
            m_transition.timer = 0.0f;
            m_transition.active = true;
        }
        else {
            m_transition = {};
        }

        m_playbackType = PlaybackType::SingleClip;
        m_currentState = nextState;
    }

    /// @brief 1D BlendTreeを再生する。再生中の同じTreeへはparameterだけを反映する
    void PlayBlendTree1D(
        const std::vector<AnimationBlendTree1DNode>& nodes,
        float parameter,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false)
    {
        if (nodes.empty()) return;

        std::vector<AnimationBlendTree1DNode> sortedNodes = nodes;
        std::sort(sortedNodes.begin(), sortedNodes.end(),
            [](const AnimationBlendTree1DNode& lhs, const AnimationBlendTree1DNode& rhs) {
                return lhs.threshold < rhs.threshold;
            });

        const bool sameTree =
            m_playbackType == PlaybackType::BlendTree1D &&
            HasSameBlendTreeNodes(sortedNodes);

        if (!restart && sameTree && !m_blendTree1DState.finished) {
            m_blendTree1DState.parameter = parameter;
            m_blendTree1DState.speed = speed;
            m_blendTree1DState.loop = loop;
            return;
        }

        m_playbackType = PlaybackType::BlendTree1D;
        m_transition = {};
        m_blendTree1DState.nodes = std::move(sortedNodes);
        m_blendTree1DState.parameter = parameter;
        m_blendTree1DState.normalizedTime = 0.0f;
        m_blendTree1DState.speed = speed;
        m_blendTree1DState.loop = loop;
        m_blendTree1DState.finished = false;
    }

    void SetBlendTree1DParameter(float parameter) {
        if (m_playbackType != PlaybackType::BlendTree1D) return;
        m_blendTree1DState.parameter = parameter;
    }

    void SetAnimationState(
        int clipIndex,
        float speed,
        float transitionTime = 0.0f)
    {
        PlayAnimation(clipIndex, speed, true, false, transitionTime);
    }
    AnimationState& GetAnimationState() { return m_currentState; }
    const AnimationState& GetAnimationState() const { return m_currentState; }
    AnimationBlendTree1DState& GetBlendTree1DState() { return m_blendTree1DState; }
    const AnimationBlendTree1DState& GetBlendTree1DState() const { return m_blendTree1DState; }
    PlaybackType GetPlaybackType() const { return m_playbackType; }
    AnimationTransition& GetTransitionState() { return m_transition; }
    const AnimationTransition& GetTransitionState() const { return m_transition; }
    bool IsTransitioning() const { return m_transition.active; }
    void CompleteTransition() { m_transition = {}; }

    bool IsFinished() const {
        if (m_transition.active) return false;
        return m_playbackType == PlaybackType::BlendTree1D
            ? m_blendTree1DState.finished
            : m_currentState.finished;
    }

private:
    bool HasSameBlendTreeNodes(const std::vector<AnimationBlendTree1DNode>& nodes) const
    {
        if (m_blendTree1DState.nodes.size() != nodes.size()) return false;

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (m_blendTree1DState.nodes[i].clipIndex != nodes[i].clipIndex ||
                m_blendTree1DState.nodes[i].threshold != nodes[i].threshold) {
                return false;
            }
        }
        return true;
    }

};

#endif // ANIMATION_COMPONENT_H
