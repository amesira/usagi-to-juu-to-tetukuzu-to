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
    int     clipIndex = -1;
    float   timer = 0.0f;
    float   speed = 1.0f; // 再生速度の倍率
    bool    loop = true;
    bool    finished = false;
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

enum class AnimationPlaybackType {
    SingleClip,
    BlendTree1D,
    BlendTree2D,
};

/// @brief Transitionを含まない、単一Clip・BlendTree共通の再生内容
struct AnimationPlaybackContent {
    AnimationPlaybackType playbackType = AnimationPlaybackType::SingleClip;
    AnimationState singleClipState;
    AnimationBlendTree1DState blendTree1DState;
    AnimationBlendTree2DState blendTree2DState;
};

/// @brief 任意の再生内容から別の再生内容へのトランジション
struct AnimationTransition {
    AnimationPlaybackContent source;
    float duration = 0.0f;
    float timer = 0.0f;
    bool active = false;
};

/// @brief 単一Clip・BlendTreeで共通利用する再生状態
struct AnimationPlaybackState : AnimationPlaybackContent {
    AnimationTransition transition;
};

/// @brief ボーンごとのレイヤー適用率。0でベース姿勢、1でレイヤー姿勢になる
struct AnimationBoneMask {
    std::vector<float> boneWeights;
};

/// @brief ベースアニメーションへOverride合成するアニメーションレイヤー
struct AnimationLayer {
    AnimationPlaybackState playbackState;
    AnimationBoneMask mask;
    float weight = 1.0f;
    bool enabled = true;
};

class AnimationComponent : public Component {
public:
    using PlaybackType = AnimationPlaybackType;

private:
    bool m_paused = false;
    AnimationPlaybackState m_playbackState;
    std::vector<AnimationLayer> m_animationLayers;

public:
    void SetPaused(bool paused) { m_paused = paused; }
    bool IsPaused() const { return m_paused; }

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

    static AnimationBoneMask CreateBoneMask(
        const ModelResource& modelResource,
        const std::vector<std::string>& boneNames,
        float weight = 1.0f)
    {
        AnimationBoneMask mask;
        mask.boneWeights.resize(modelResource.bones.size(), 0.0f);

        weight = std::clamp(weight, 0.0f, 1.0f);

        for (const std::string& boneName : boneNames) {
            const auto rootIt = modelResource.boneNameToIndex.find(boneName);
            if (rootIt == modelResource.boneNameToIndex.end()) continue;

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
        bool restart = false,
        float transitionTime = 0.0f)
    {
        if (layerIndex >= m_animationLayers.size() || clipIndex < 0) return false;

        AnimationLayer& layer = m_animationLayers[layerIndex];
        AnimationPlaybackState& playback = layer.playbackState;
        AnimationState& state = playback.singleClipState;
        if (!restart &&
            layer.enabled &&
            playback.playbackType == PlaybackType::SingleClip &&
            state.clipIndex == clipIndex &&
            !state.finished) {
            state.speed = speed;
            state.loop = loop;
            return true;
        }

        BeginTransition(playback, transitionTime);
        playback.playbackType = PlaybackType::SingleClip;
        state = { clipIndex, 0.0f, speed, loop, false };
        layer.enabled = true;
        return true;
    }

    bool PlayLayerBlendTree1D(
        size_t layerIndex,
        const std::vector<AnimationBlendTree1DNode>& nodes,
        float parameter,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false,
        float transitionTime = 0.0f)
    {
        if (layerIndex >= m_animationLayers.size() || nodes.empty()) return false;

        std::vector<AnimationBlendTree1DNode> sortedNodes = nodes;
        SortBlendTree1DNodes(sortedNodes);

        AnimationLayer& layer = m_animationLayers[layerIndex];
        AnimationPlaybackState& playback = layer.playbackState;
        AnimationBlendTree1DState& state = playback.blendTree1DState;
        const bool sameTree =
            playback.playbackType == PlaybackType::BlendTree1D &&
            HasSameBlendTreeNodes(state.nodes, sortedNodes);

        if (!restart && sameTree && !state.finished) {
            state.parameter = parameter;
            state.speed = speed;
            state.loop = loop;
            layer.enabled = true;
            return true;
        }

        BeginTransition(playback, transitionTime);
        playback.playbackType = PlaybackType::BlendTree1D;
        state.nodes = std::move(sortedNodes);
        state.parameter = parameter;
        state.speed = speed;
        state.loop = loop;
        state.finished = false;
        layer.enabled = true;
        return true;
    }

    bool PlayLayerBlendTree2D(
        size_t layerIndex,
        const std::vector<AnimationBlendTree2DNode>& nodes,
        const DirectX::XMFLOAT2& parameter,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false,
        float transitionTime = 0.0f)
    {
        if (layerIndex >= m_animationLayers.size() || nodes.empty()) return false;

        AnimationLayer& layer = m_animationLayers[layerIndex];
        AnimationPlaybackState& playback = layer.playbackState;
        AnimationBlendTree2DState& state = playback.blendTree2DState;
        const bool sameTree =
            playback.playbackType == PlaybackType::BlendTree2D &&
            HasSameBlendTree2DNodes(state.nodes, nodes);

        if (!restart && sameTree && !state.finished) {
            state.parameter = parameter;
            state.speed = speed;
            state.loop = loop;
            layer.enabled = true;
            return true;
        }

        BeginTransition(playback, transitionTime);
        playback.playbackType = PlaybackType::BlendTree2D;
        state.nodes = nodes;
        state.parameter = parameter;
        state.speed = speed;
        state.loop = loop;
        state.finished = false;
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
            IsPlaybackFinished(m_animationLayers[layerIndex].playbackState);
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
        AnimationState& currentState = m_playbackState.singleClipState;
        if (!restart &&
            m_playbackState.playbackType == PlaybackType::SingleClip &&
            currentState.clipIndex == clipIndex &&
            !currentState.finished) return;

        AnimationState nextState;
        nextState.clipIndex = clipIndex;
        nextState.timer = 0.0f;
        nextState.speed = speed;
        nextState.loop = loop;
        nextState.finished = false;

        BeginTransition(m_playbackState, transitionTime);

        m_playbackState.playbackType = PlaybackType::SingleClip;
        currentState = nextState;
    }

    /// @brief 1D BlendTreeを再生する。再生中の同じTreeへはparameterだけを反映する
    void PlayBlendTree1D(
        const std::vector<AnimationBlendTree1DNode>& nodes,
        float parameter,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false,
        float transitionTime = 0.0f)
    {
        if (nodes.empty()) return;

        std::vector<AnimationBlendTree1DNode> sortedNodes = nodes;
        SortBlendTree1DNodes(sortedNodes);

        const bool sameTree =
            m_playbackState.playbackType == PlaybackType::BlendTree1D &&
            HasSameBlendTreeNodes(m_playbackState.blendTree1DState.nodes, sortedNodes);

        AnimationBlendTree1DState& state = m_playbackState.blendTree1DState;
        if (!restart && sameTree && !state.finished) {
            state.parameter = parameter;
            state.speed = speed;
            state.loop = loop;
            return;
        }

        BeginTransition(m_playbackState, transitionTime);
        m_playbackState.playbackType = PlaybackType::BlendTree1D;
        state.nodes = std::move(sortedNodes);
        state.parameter = parameter;
        state.normalizedTime = 0.0f;
        state.speed = speed;
        state.loop = loop;
        state.finished = false;
    }

    void SetBlendTree1DParameter(float parameter) {
        if (m_playbackState.playbackType != PlaybackType::BlendTree1D) return;
        m_playbackState.blendTree1DState.parameter = parameter;
    }

    /// @brief 2D BlendTreeを再生する。同じTreeの再要求ではparameterだけを更新する
    void PlayBlendTree2D(
        const std::vector<AnimationBlendTree2DNode>& nodes,
        const DirectX::XMFLOAT2& parameter,
        float speed = 1.0f,
        bool loop = true,
        bool restart = false,
        float transitionTime = 0.0f)
    {
        if (nodes.empty()) return;

        const bool sameTree =
            m_playbackState.playbackType == PlaybackType::BlendTree2D &&
            HasSameBlendTree2DNodes(m_playbackState.blendTree2DState.nodes, nodes);

        AnimationBlendTree2DState& state = m_playbackState.blendTree2DState;
        if (!restart && sameTree && !state.finished) {
            state.parameter = parameter;
            state.speed = speed;
            state.loop = loop;
            return;
        }

        BeginTransition(m_playbackState, transitionTime);
        m_playbackState.playbackType = PlaybackType::BlendTree2D;
        state.nodes = nodes;
        state.parameter = parameter;
        state.normalizedTime = 0.0f;
        state.speed = speed;
        state.loop = loop;
        state.finished = false;
    }

    void SetBlendTree2DParameter(const DirectX::XMFLOAT2& parameter) {
        if (m_playbackState.playbackType != PlaybackType::BlendTree2D) return;
        m_playbackState.blendTree2DState.parameter = parameter;
    }

    void SetAnimationState(
        int clipIndex,
        float speed,
        float transitionTime = 0.0f)
    {
        PlayAnimation(clipIndex, speed, true, false, transitionTime);
    }
    AnimationPlaybackState& GetPlaybackState() { return m_playbackState; }
    const AnimationPlaybackState& GetPlaybackState() const { return m_playbackState; }
    AnimationState& GetAnimationState() { return m_playbackState.singleClipState; }
    const AnimationState& GetAnimationState() const { return m_playbackState.singleClipState; }
    AnimationBlendTree1DState& GetBlendTree1DState() { return m_playbackState.blendTree1DState; }
    const AnimationBlendTree1DState& GetBlendTree1DState() const { return m_playbackState.blendTree1DState; }
    AnimationBlendTree2DState& GetBlendTree2DState() { return m_playbackState.blendTree2DState; }
    const AnimationBlendTree2DState& GetBlendTree2DState() const { return m_playbackState.blendTree2DState; }
    PlaybackType GetPlaybackType() const { return m_playbackState.playbackType; }
    AnimationTransition& GetTransitionState() { return m_playbackState.transition; }
    const AnimationTransition& GetTransitionState() const { return m_playbackState.transition; }
    bool IsTransitioning() const { return m_playbackState.transition.active; }
    void CompleteTransition() { m_playbackState.transition = {}; }

    bool IsFinished() const {
        return IsPlaybackFinished(m_playbackState);
    }

private:
    static void BeginTransition(AnimationPlaybackState& playback, float transitionTime)
    {
        if (transitionTime <= 0.0f || !HasValidPlayback(playback)) {
            playback.transition = {};
            return;
        }

        // 遷移中の再要求では、現在の遷移先を新しい遷移元として扱う。
        playback.transition.source = static_cast<const AnimationPlaybackContent&>(playback);
        playback.transition.duration = transitionTime;
        playback.transition.timer = 0.0f;
        playback.transition.active = true;
    }

    static bool HasValidPlayback(const AnimationPlaybackContent& playback)
    {
        switch (playback.playbackType) {
        case PlaybackType::SingleClip:
            return playback.singleClipState.clipIndex >= 0;
        case PlaybackType::BlendTree1D:
            return !playback.blendTree1DState.nodes.empty();
        case PlaybackType::BlendTree2D:
            return !playback.blendTree2DState.nodes.empty();
        default:
            return false;
        }
    }

    static void SortBlendTree1DNodes(std::vector<AnimationBlendTree1DNode>& nodes)
    {
        std::sort(nodes.begin(), nodes.end(),
            [](const AnimationBlendTree1DNode& lhs, const AnimationBlendTree1DNode& rhs) {
                return lhs.threshold < rhs.threshold;
            });
    }

    static bool HasSameBlendTreeNodes(
        const std::vector<AnimationBlendTree1DNode>& currentNodes,
        const std::vector<AnimationBlendTree1DNode>& nodes)
    {
        if (currentNodes.size() != nodes.size()) return false;

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (currentNodes[i].clipIndex != nodes[i].clipIndex ||
                currentNodes[i].threshold != nodes[i].threshold) {
                return false;
            }
        }
        return true;
    }

    static bool HasSameBlendTree2DNodes(
        const std::vector<AnimationBlendTree2DNode>& currentNodes,
        const std::vector<AnimationBlendTree2DNode>& nodes)
    {
        if (currentNodes.size() != nodes.size()) return false;

        for (size_t i = 0; i < nodes.size(); ++i) {
            const AnimationBlendTree2DNode& current = currentNodes[i];
            if (current.clipIndex != nodes[i].clipIndex ||
                current.threshold.x != nodes[i].threshold.x ||
                current.threshold.y != nodes[i].threshold.y) {
                return false;
            }
        }
        return true;
    }

    static bool IsPlaybackFinished(const AnimationPlaybackState& playback)
    {
        if (playback.transition.active) return false;
        if (playback.playbackType == PlaybackType::BlendTree1D) {
            return playback.blendTree1DState.finished;
        }
        if (playback.playbackType == PlaybackType::BlendTree2D) {
            return playback.blendTree2DState.finished;
        }
        return playback.singleClipState.finished;
    }

};

#endif // ANIMATION_COMPONENT_H
