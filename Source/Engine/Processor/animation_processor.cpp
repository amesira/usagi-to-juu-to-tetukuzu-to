//===================================================
// animation_processor.cpp
// 
// Author：Miu Kitamura
// Date  ：2026/04/06
//===================================================
#include "animation_processor.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"
#include "Engine/Device/mi_fps.h"
#include "Utility/mi_math.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/animation_component.h"

void AnimationProcessor::Initialize() {}
void AnimationProcessor::Finalize() {}

void AnimationProcessor::Process(IScene* pScene)
{
    auto* animationPool = pScene->GetComponentPool<AnimationComponent>();
    auto* modelPool = pScene->GetComponentPool<ModelComponent>();
    if (!animationPool || !modelPool) return;

    const float deltaTime = FPS_GetDeltaTime();
    for (AnimationComponent& animationComponent : animationPool->GetList()) {
        if (!animationComponent.GetEnable()) continue;

        ModelComponent* modelComponent =
            modelPool->GetByGameObjectID(animationComponent.GetOwner()->GetID());
        if (!modelComponent || !modelComponent->GetEnable()) continue;

        ModelResource* modelResource = modelComponent->GetModelResource();
        if (!modelResource || modelResource->animationClips.empty()) continue;

        // PlaybackStateを評価して、LocalPoseを計算する
        LocalPose resultPose = MakeDefaultLocalPose(modelResource->defaultPose);
        LocalPose evaluatedPose;
        if (EvaluatePlaybackState(
            evaluatedPose,
            animationComponent.GetPlaybackState(),
            *modelResource,
            resultPose,
            deltaTime)) {
            resultPose = std::move(evaluatedPose);
        }

        // AnimationLayerを評価して、LocalPoseをブレンドする
        for (AnimationLayer& layer : animationComponent.GetAnimationLayers()) {
            if (!layer.enabled || layer.weight <= 0.0f) continue;

            LocalPose layerPose;
            if (!EvaluatePlaybackState(
                layerPose,
                layer.playbackState,
                *modelResource,
                resultPose,
                deltaTime)) {
                continue;
            }

            resultPose = BlendLocalPosesMasked(
                resultPose,
                layerPose,
                layer.mask,
                layer.weight);
        }

        // LocalPoseをSkeletonPoseに適用して、ボーン行列を計算する
        SkeletonPose pose = modelResource->defaultPose;
        ApplyLocalPose(pose, resultPose);
        BuildSkeletonMatrices(pose, *modelResource);
        modelComponent->SetSkeletonPose(pose);
    }
}

#pragma region Animation Evaluation
bool AnimationProcessor::EvaluatePlaybackState(
    LocalPose& outPose,
    AnimationPlaybackState& playbackState,
    ModelResource& modelResource,
    const LocalPose& fallbackPose,
    float deltaTime)
{
    if (playbackState.transition.active &&
        playbackState.playbackType == AnimationPlaybackType::SingleClip) {
        return EvaluateTransition(
            outPose,
            playbackState,
            modelResource,
            fallbackPose,
            deltaTime);
    }

    switch (playbackState.playbackType) {
    case AnimationPlaybackType::SingleClip:
        return EvaluateSingleClip(
            outPose,
            playbackState.singleClipState,
            modelResource,
            fallbackPose,
            deltaTime);
    case AnimationPlaybackType::BlendTree1D:
        return EvaluateBlendTree1D(
            outPose,
            playbackState.blendTree1DState,
            modelResource,
            fallbackPose,
            deltaTime);
    case AnimationPlaybackType::BlendTree2D:
        return EvaluateBlendTree2D(
            outPose,
            playbackState.blendTree2DState,
            modelResource,
            fallbackPose,
            deltaTime);
    default:
        return false;
    }
}

bool AnimationProcessor::EvaluateSingleClip(
    LocalPose& outPose,
    AnimationState& state,
    ModelResource& modelResource,
    const LocalPose& fallbackPose,
    float deltaTime)
{
    const AnimationClip* clip = ResolveAnimationClip(state, modelResource);
    if (!clip) return false;

    const float animationTime = AdvanceAnimationState(state, *clip, deltaTime);
    outPose = SampleLocalPose(*clip, fallbackPose, animationTime);
    return true;
}

bool AnimationProcessor::EvaluateTransition(
    LocalPose& outPose,
    AnimationPlaybackState& playbackState,
    ModelResource& modelResource,
    const LocalPose& fallbackPose,
    float deltaTime)
{
    AnimationTransition& transition = playbackState.transition;
    AnimationState& destinationState = playbackState.singleClipState;
    const AnimationClip* sourceClip = ResolveAnimationClip(transition.sourceState, modelResource);
    const AnimationClip* destinationClip = ResolveAnimationClip(destinationState, modelResource);

    if (!sourceClip || !destinationClip || transition.duration <= 0.0f) {
        transition = {};
        return EvaluateSingleClip(
            outPose,
            destinationState,
            modelResource,
            fallbackPose,
            deltaTime);
    }

    const float sourceTime = AdvanceAnimationState(
        transition.sourceState, *sourceClip, deltaTime);
    const float destinationTime = AdvanceAnimationState(
        destinationState, *destinationClip, deltaTime);

    transition.timer += deltaTime;
    float weight = MiMath::Clamp(transition.timer / transition.duration, 0.0f, 1.0f);
    weight = weight * weight * (3.0f - 2.0f * weight);

    const LocalPose sourcePose = SampleLocalPose(*sourceClip, fallbackPose, sourceTime);
    const LocalPose destinationPose = SampleLocalPose(
        *destinationClip, fallbackPose, destinationTime);
    outPose = BlendLocalPoses(sourcePose, destinationPose, weight);

    if (transition.timer >= transition.duration) transition = {};
    return true;
}

bool AnimationProcessor::EvaluateBlendTree1D(
    LocalPose& outPose,
    AnimationBlendTree1DState& state,
    ModelResource& modelResource,
    const LocalPose& fallbackPose,
    float deltaTime)
{

    // 有効なノードを収集する
    std::vector<const AnimationBlendTree1DNode*> validNodes;
    validNodes.reserve(state.nodes.size());
    for (const AnimationBlendTree1DNode& node : state.nodes) 
    {
        if (node.clipIndex < 0 || node.clipIndex >= static_cast<int>(modelResource.animationClips.size())) continue;
        if (modelResource.animationClips[node.clipIndex].duration <= 0.0f) continue;
        validNodes.push_back(&node);
    }
    if (validNodes.empty()) return false;

    size_t lowerIndex = 0;
    size_t upperIndex = 0;
    float blendWeight = 0.0f;
    // 閾値を超えれば単一のノードを使用する
    if (state.parameter <= validNodes.front()->threshold) {
        lowerIndex = upperIndex = 0;
    }
    else if (state.parameter >= validNodes.back()->threshold) {
        lowerIndex = upperIndex = validNodes.size() - 1;
    }
    else {
        for (size_t i = 1; i < validNodes.size(); ++i) {
            if (state.parameter <= validNodes[i]->threshold) {
                lowerIndex = i - 1;
                upperIndex = i;
                const float range = validNodes[upperIndex]->threshold - validNodes[lowerIndex]->threshold;
                if (range > 0.0f) {
                    blendWeight = (state.parameter - validNodes[lowerIndex]->threshold) / range;
                }
                break;
            }
        }
    }

    // 2つのクリップの再生時間を補間するために、ブレンドされたクリップの長さを計算する
    const AnimationClip& lowerClip = modelResource.animationClips[validNodes[lowerIndex]->clipIndex];
    const AnimationClip& upperClip = modelResource.animationClips[validNodes[upperIndex]->clipIndex];
    const float blendedDuration = MiMath::Lerp(lowerClip.duration, upperClip.duration, blendWeight);

    if (!state.finished && blendedDuration > 0.0f) {
        state.normalizedTime += deltaTime * state.speed / blendedDuration;
    }

    float normalizedTime = state.normalizedTime;
    if (state.loop) {
        normalizedTime = std::fmod(normalizedTime, 1.0f);
        if (normalizedTime < 0.0f) normalizedTime += 1.0f;
        state.normalizedTime = normalizedTime;
    }
    else {
        normalizedTime = MiMath::Clamp(normalizedTime, 0.0f, 1.0f);
        if (state.normalizedTime >= 1.0f) {
            state.normalizedTime = 1.0f;
            state.finished = true;
        }
    }

    const LocalPose lowerPose = SampleLocalPose(
        lowerClip, fallbackPose, normalizedTime * lowerClip.duration);

    LocalPose resultPose = lowerPose;
    if (lowerIndex != upperIndex) {
        const LocalPose upperPose = SampleLocalPose(
            upperClip, fallbackPose, normalizedTime * upperClip.duration);
        resultPose = BlendLocalPoses(lowerPose, upperPose, blendWeight);
    }

    outPose = std::move(resultPose);
    return true;
}

/// @brief 2Dパラメータに近い最大3ノードを距離加重してBlendTreeを再生する
bool AnimationProcessor::EvaluateBlendTree2D(
    LocalPose& outPose,
    AnimationBlendTree2DState& state,
    ModelResource& modelResource,
    const LocalPose& fallbackPose,
    float deltaTime)
{
    struct WeightedNode {
        const AnimationBlendTree2DNode* node = nullptr;
        float distanceSquared = 0.0f;
        float weight = 0.0f;
    };

    std::vector<WeightedNode> candidates;
    candidates.reserve(state.nodes.size());
    for (const AnimationBlendTree2DNode& node : state.nodes) {
        if (node.clipIndex < 0 ||
            node.clipIndex >= static_cast<int>(modelResource.animationClips.size())) {
            continue;
        }
        if (modelResource.animationClips[node.clipIndex].duration <= 0.0f) continue;

        const float deltaX = state.parameter.x - node.threshold.x;
        const float deltaY = state.parameter.y - node.threshold.y;
        candidates.push_back({ &node, deltaX * deltaX + deltaY * deltaY, 0.0f });
    }
    if (candidates.empty()) return false;

    std::sort(candidates.begin(), candidates.end(),
        [](const WeightedNode& lhs, const WeightedNode& rhs) {
            return lhs.distanceSquared < rhs.distanceSquared;
        });
    if (candidates.size() > 3) candidates.resize(3);

    constexpr float EXACT_NODE_EPSILON = 0.000001f;
    if (candidates.front().distanceSquared <= EXACT_NODE_EPSILON) {
        candidates.front().weight = 1.0f;
        candidates.resize(1);
    }
    else {
        float totalWeight = 0.0f;
        for (WeightedNode& candidate : candidates) {
            candidate.weight = 1.0f / candidate.distanceSquared;
            totalWeight += candidate.weight;
        }
        if (totalWeight <= 0.0f) return false;
        for (WeightedNode& candidate : candidates) {
            candidate.weight /= totalWeight;
        }
    }

    float blendedDuration = 0.0f;
    for (const WeightedNode& candidate : candidates) {
        blendedDuration +=
            modelResource.animationClips[candidate.node->clipIndex].duration *
            candidate.weight;
    }
    if (!state.finished && blendedDuration > 0.0f) {
        state.normalizedTime += deltaTime * state.speed / blendedDuration;
    }

    float normalizedTime = state.normalizedTime;
    if (state.loop) {
        normalizedTime = std::fmod(normalizedTime, 1.0f);
        if (normalizedTime < 0.0f) normalizedTime += 1.0f;
        state.normalizedTime = normalizedTime;
    }
    else {
        normalizedTime = MiMath::Clamp(normalizedTime, 0.0f, 1.0f);
        if (state.normalizedTime >= 1.0f) {
            state.normalizedTime = 1.0f;
            state.finished = true;
        }
    }

    float accumulatedWeight = candidates.front().weight;
    const AnimationClip& firstClip =
        modelResource.animationClips[candidates.front().node->clipIndex];
    LocalPose resultPose = SampleLocalPose(
        firstClip,
        fallbackPose,
        normalizedTime * firstClip.duration);

    for (size_t i = 1; i < candidates.size(); ++i) {
        const AnimationClip& clip =
            modelResource.animationClips[candidates[i].node->clipIndex];
        const LocalPose nodePose = SampleLocalPose(
            clip,
            fallbackPose,
            normalizedTime * clip.duration);

        const float nextAccumulatedWeight = accumulatedWeight + candidates[i].weight;
        const float blendWeight = nextAccumulatedWeight > 0.0f
            ? candidates[i].weight / nextAccumulatedWeight
            : 0.0f;
        resultPose = BlendLocalPoses(resultPose, nodePose, blendWeight);
        accumulatedWeight = nextAccumulatedWeight;
    }

    outPose = std::move(resultPose);
    return true;
}
#pragma endregion

/// @brief AnimationStateが参照するクリップを取得する
const AnimationClip* AnimationProcessor::ResolveAnimationClip(
    AnimationState& state,
    ModelResource& modelResource)
{
    if (state.clipIndex >= 0 &&
        state.clipIndex < static_cast<int>(modelResource.animationClips.size())) 
    {
        const AnimationClip& clip = modelResource.animationClips[state.clipIndex];
        return clip.duration > 0.0f ? &clip : nullptr;
    }

    return nullptr;
}

/// @brief AnimationStateの時間を進め、サンプリングに使用する時間を返す
float AnimationProcessor::AdvanceAnimationState(
    AnimationState& state,
    const AnimationClip& clip,
    float deltaTime)
{
    if (!state.finished) state.timer += deltaTime * state.speed;

    if (state.loop) {
        float animationTime = std::fmod(state.timer, clip.duration);
        if (animationTime < 0.0f) animationTime += clip.duration;
        return animationTime;
    }

    const float animationTime = MiMath::Clamp(state.timer, 0.0f, clip.duration);
    if (state.timer >= clip.duration) {
        state.timer = clip.duration;
        state.finished = true;
    }
    return animationTime;
}

/// @brief アニメーションクリップから指定された時間のローカルポーズをサンプリングする
AnimationProcessor::LocalPose AnimationProcessor::SampleLocalPose(
    const AnimationClip& clip,
    const LocalPose& fallbackPose,
    float animationTime)
{
    LocalPose result = fallbackPose;

    for (const AnimationClip::AnimationChannel& channel : clip.channels) {
        const unsigned int boneIndex = channel.boneIndex;
        if (boneIndex >= result.positions.size()) continue;

        if (!channel.positionKeyframes.empty()) {
            result.positions[boneIndex] = SamplingKeyframes(channel.positionKeyframes, animationTime);
        }
        if (!channel.rotationKeyframes.empty()) {
            result.rotations[boneIndex] = SamplingKeyframes(channel.rotationKeyframes, animationTime, true);
        }
        if (!channel.scalingKeyframes.empty()) {
            result.scales[boneIndex] = SamplingKeyframes(channel.scalingKeyframes, animationTime);
        }
    }
    return result;
}

/// @brief 2つのローカルポーズをブレンドする
AnimationProcessor::LocalPose AnimationProcessor::BlendLocalPoses(
    const LocalPose& lhs,
    const LocalPose& rhs,
    float weight)
{
    LocalPose result = lhs;
    const size_t boneCount = (std::min)(lhs.positions.size(), rhs.positions.size());
    weight = MiMath::Clamp(weight, 0.0f, 1.0f);

    for (size_t i = 0; i < boneCount; ++i) {
        result.positions[i] = MiMath::Lerp(lhs.positions[i], rhs.positions[i], weight);
        result.scales[i] = MiMath::Lerp(lhs.scales[i], rhs.scales[i], weight);

        XMVECTOR rotation = XMQuaternionSlerp(
            XMLoadFloat4(&lhs.rotations[i]),
            XMLoadFloat4(&rhs.rotations[i]),
            weight);
        XMStoreFloat4(&result.rotations[i], XMQuaternionNormalize(rotation));
    }
    return result;
}

/// @brief ボーンマスクを適用してレイヤー姿勢をOverride合成する
AnimationProcessor::LocalPose AnimationProcessor::BlendLocalPosesMasked(
    const LocalPose& basePose,
    const LocalPose& layerPose,
    const AnimationBoneMask& mask,
    float layerWeight)
{
    LocalPose result = basePose;
    const size_t boneCount = (std::min)({
        basePose.positions.size(),
        layerPose.positions.size(),
        mask.boneWeights.size(),
    });
    layerWeight = MiMath::Clamp(layerWeight, 0.0f, 1.0f);

    for (size_t i = 0; i < boneCount; ++i) {
        const float weight = MiMath::Clamp(
            mask.boneWeights[i] * layerWeight,
            0.0f,
            1.0f);
        if (weight <= 0.0f) continue;

        result.positions[i] = MiMath::Lerp(
            basePose.positions[i], layerPose.positions[i], weight);
        result.scales[i] = MiMath::Lerp(
            basePose.scales[i], layerPose.scales[i], weight);

        const XMVECTOR rotation = XMQuaternionSlerp(
            XMLoadFloat4(&basePose.rotations[i]),
            XMLoadFloat4(&layerPose.rotations[i]),
            weight);
        XMStoreFloat4(&result.rotations[i], XMQuaternionNormalize(rotation));
    }
    return result;
}

/// @brief SkeletonPoseのデフォルトTRSからローカルポーズを作成する
AnimationProcessor::LocalPose AnimationProcessor::MakeDefaultLocalPose(const SkeletonPose& pose)
{
    return {
        pose.defaultPositions,
        pose.defaultRotations,
        pose.defaultScales,
    };
}

/// @brief ローカルポーズをSkeletonPoseに適用する
void AnimationProcessor::ApplyLocalPose(SkeletonPose& pose, const LocalPose& localPose)
{
    const size_t boneCount = (std::min)(pose.localTransforms.size(), localPose.positions.size());
    for (size_t i = 0; i < boneCount; ++i) {
        const XMFLOAT4& position = localPose.positions[i];
        const XMFLOAT4& rotation = localPose.rotations[i];
        const XMFLOAT4& scaling = localPose.scales[i];
        pose.localTransforms[i] =
            XMMatrixScaling(scaling.x, scaling.y, scaling.z) *
            XMMatrixRotationQuaternion(XMLoadFloat4(&rotation)) *
            XMMatrixTranslation(position.x, position.y, position.z);
    }
}

/// @brief SkeletonPoseのローカル変換からグローバル変換とボーン変換を計算する
void AnimationProcessor::BuildSkeletonMatrices(
    SkeletonPose& pose,
    const ModelResource& modelResource)
{
    if (modelResource.rootBoneIndex >= modelResource.bones.size()) return;

    std::function<void(unsigned int, const XMMATRIX&)> calculateGlobalTransform =
        [&](unsigned int index, const XMMATRIX& parentTransform) {
            if (index >= pose.localTransforms.size()) return;
            const XMMATRIX globalTransform = pose.localTransforms[index] * parentTransform;
            pose.globalTransforms[index] = globalTransform;
            for (unsigned int childIndex : modelResource.bones[index].childIndices) {
                calculateGlobalTransform(childIndex, globalTransform);
            }
        };

    calculateGlobalTransform(modelResource.rootBoneIndex, modelResource.rootParentCorrection);

    const size_t boneCount = (std::min)(pose.boneTransforms.size(), modelResource.bones.size());
    for (size_t i = 0; i < boneCount; ++i) {
        pose.boneTransforms[i] = modelResource.bones[i].offsetMatrix * pose.globalTransforms[i];
    }
}

/// @brief 指定された時間におけるキーフレームの値をサンプリングする
XMFLOAT4 AnimationProcessor::SamplingKeyframes(
    const std::vector<AnimationClip::Keyframe>& keyframes,
    float time,
    bool useQuaternionSlerp)
{
    if (keyframes.empty()) return XMFLOAT4(0, 0, 0, 1);
    if (keyframes.size() == 1) return keyframes.front().keyValue;

    size_t previousIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < keyframes.size(); ++i) {
        if (keyframes[i].time > time) {
            nextIndex = i;
            break;
        }
        previousIndex = i;
    }
    if (nextIndex == 0) nextIndex = keyframes.size() - 1;

    if (previousIndex == nextIndex ||
        keyframes[nextIndex].time == keyframes[previousIndex].time) {
        return keyframes[previousIndex].keyValue;
    }

    float weight =
        (time - keyframes[previousIndex].time) /
        (keyframes[nextIndex].time - keyframes[previousIndex].time);
    weight = MiMath::Clamp(weight, 0.0f, 1.0f);

    if (!useQuaternionSlerp) {
        return MiMath::Lerp(
            keyframes[previousIndex].keyValue,
            keyframes[nextIndex].keyValue,
            weight);
    }

    const XMVECTOR rotation = XMQuaternionSlerp(
        XMLoadFloat4(&keyframes[previousIndex].keyValue),
        XMLoadFloat4(&keyframes[nextIndex].keyValue),
        weight);
    XMFLOAT4 result;
    XMStoreFloat4(&result, XMQuaternionNormalize(rotation));
    return result;
}
