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
    int         clipIndex = -1;
    float   timer = 0.0f;
    float   speed = 1.0f; // 再生速度の倍率
    bool    loop = true;
    bool    finished = false;
};

// アニメーションのトランジションを表す構造体
struct AnimationTransition {
    AnimationState sourceState; // 遷移元のアニメーション状態
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

/// @brief 2D BlendTreeを構成するアニメーションクリップ
struct AnimationBlendTree2DNode {
    int clipIndex = -1;
    DirectX::XMFLOAT2 threshold = {};
};

/// @brief 2D BlendTreeの再生状態
struct AnimationBlendTree2DState {
    std::vector<AnimationBlendTree2DNode> nodes;
    DirectX::XMFLOAT2 parameter = {};
    float normalizedTime = 0.0f;
    float speed = 1.0f;
    bool loop = true;
    bool finished = false;
};

/// @brief ボーンごとのレイヤー適用率。0でベース姿勢、1でレイヤー姿勢になる
struct AnimationBoneMask {
    std::vector<float> boneWeights;
};

/// @brief ベースアニメーションへOverride合成するアニメーションレイヤー
struct AnimationLayer {
    AnimationState state;
    AnimationBoneMask mask;
    float weight = 1.0f;
    bool enabled = true;
};

class AnimationComponent : public Component {
public:
    enum class PlaybackType {
        SingleClip,
        BlendTree1D,
        BlendTree2D,
    };

private:
    AnimationState m_currentState; // 現在のアニメーション状態
    AnimationTransition m_transition;
    AnimationBlendTree1DState m_blendTree1DState;
    AnimationBlendTree2DState m_blendTree2DState;
    std::vector<AnimationLayer> m_animationLayers;
    PlaybackType m_playbackType = PlaybackType::SingleClip;

public:
    static constexpr char CLIP_NONE[] = "None";

    /// @brief 指定ボーンとその全子ボーンを有効にしたマスクを作成する
    static AnimationBoneMask CreateBoneMask(
        const ModelResource& modelResource,
        const std::string& rootBoneName,
        float weight = 1.0f)
    {
        AnimationBoneMask mask;
        mask.boneWeights.resize(modelResource.bones.size(), 0.0f);

        const auto rootIt = modelResource.boneNameToIndex.find(rootBoneName);
        if (rootIt == modelResource.boneNameToIndex.end()) return mask;

        weight = std::clamp(weight, 0.0f, 1.0f);
        std::vector<unsigned int> pendingBones{ rootIt->second };
        while (!pendingBones.empty()) {
            const unsigned int boneIndex = pendingBones.back();
            pendingBones.pop_back();
            if (boneIndex >= modelResource.bones.size()) continue;

            mask.boneWeights[boneIndex] = weight;
            const ModelBone& bone = modelResource.bones[boneIndex];
            pendingBones.insert(
                pendingBones.end(),
                bone.childIndices.begin(),
                bone.childIndices.end());
        }
        return mask;
    }

    /// @brief Overrideレイヤーを追加し、そのインデックスを返す
    size_t AddAnimationLayer(const AnimationBoneMask& mask, float weight = 1.0f)
    {
        AnimationLayer layer;
        layer.mask = mask;
        layer.weight = std::clamp(weight, 0.0f, 1.0f);
        m_animationLayers.emplace_back(std::move(layer));
        return m_animationLayers.size() - 1;
    }

    bool PlayLayerAnimation(
        size_t layerIndex,
        int clipIndex,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false)
    {
        if (layerIndex >= m_animationLayers.size() || clipIndex < 0) return false;

        AnimationLayer& layer = m_animationLayers[layerIndex];
        if (!restart &&
            layer.enabled &&
            layer.state.clipIndex == clipIndex &&
            !layer.state.finished) {
            layer.state.speed = speed;
            layer.state.loop = loop;
            return true;
        }

        layer.state.clipIndex = clipIndex;
        layer.state.timer = 0.0f;
        layer.state.speed = speed;
        layer.state.loop = loop;
        layer.state.finished = false;
        layer.enabled = true;
        return true;
    }

    bool StopAnimationLayer(size_t layerIndex)
    {
        if (layerIndex >= m_animationLayers.size()) return false;
        m_animationLayers[layerIndex].enabled = false;
        return true;
    }

    bool SetAnimationLayerWeight(size_t layerIndex, float weight)
    {
        if (layerIndex >= m_animationLayers.size()) return false;
        m_animationLayers[layerIndex].weight = std::clamp(weight, 0.0f, 1.0f);
        return true;
    }

    bool IsLayerAnimationFinished(size_t layerIndex) const
    {
        return layerIndex < m_animationLayers.size() &&
            m_animationLayers[layerIndex].state.finished;
    }

    std::vector<AnimationLayer>& GetAnimationLayers() { return m_animationLayers; }
    const std::vector<AnimationLayer>& GetAnimationLayers() const { return m_animationLayers; }

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

    /// @brief 2D BlendTreeを再生する。同じTreeの再要求ではparameterだけを更新する
    void PlayBlendTree2D(
        const std::vector<AnimationBlendTree2DNode>& nodes,
        const DirectX::XMFLOAT2& parameter,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false)
    {
        if (nodes.empty()) return;

        const bool sameTree =
            m_playbackType == PlaybackType::BlendTree2D &&
            HasSameBlendTree2DNodes(nodes);

        if (!restart && sameTree && !m_blendTree2DState.finished) {
            m_blendTree2DState.parameter = parameter;
            m_blendTree2DState.speed = speed;
            m_blendTree2DState.loop = loop;
            return;
        }

        m_playbackType = PlaybackType::BlendTree2D;
        m_transition = {};
        m_blendTree2DState.nodes = nodes;
        m_blendTree2DState.parameter = parameter;
        m_blendTree2DState.normalizedTime = 0.0f;
        m_blendTree2DState.speed = speed;
        m_blendTree2DState.loop = loop;
        m_blendTree2DState.finished = false;
    }

    void SetBlendTree2DParameter(const DirectX::XMFLOAT2& parameter) {
        if (m_playbackType != PlaybackType::BlendTree2D) return;
        m_blendTree2DState.parameter = parameter;
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
    AnimationBlendTree2DState& GetBlendTree2DState() { return m_blendTree2DState; }
    const AnimationBlendTree2DState& GetBlendTree2DState() const { return m_blendTree2DState; }
    PlaybackType GetPlaybackType() const { return m_playbackType; }
    AnimationTransition& GetTransitionState() { return m_transition; }
    const AnimationTransition& GetTransitionState() const { return m_transition; }
    bool IsTransitioning() const { return m_transition.active; }
    void CompleteTransition() { m_transition = {}; }

    bool IsFinished() const {
        if (m_transition.active) return false;
        if (m_playbackType == PlaybackType::BlendTree1D) {
            return m_blendTree1DState.finished;
        }
        if (m_playbackType == PlaybackType::BlendTree2D) {
            return m_blendTree2DState.finished;
        }
        return m_currentState.finished;
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

    bool HasSameBlendTree2DNodes(const std::vector<AnimationBlendTree2DNode>& nodes) const
    {
        if (m_blendTree2DState.nodes.size() != nodes.size()) return false;

        for (size_t i = 0; i < nodes.size(); ++i) {
            const AnimationBlendTree2DNode& current = m_blendTree2DState.nodes[i];
            if (current.clipIndex != nodes[i].clipIndex ||
                current.threshold.x != nodes[i].threshold.x ||
                current.threshold.y != nodes[i].threshold.y) {
                return false;
            }
        }
        return true;
    }

};

#endif // ANIMATION_COMPONENT_H
