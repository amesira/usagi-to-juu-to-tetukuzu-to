//===================================================
// animation_processor.cpp
// Author：Miu Kitamura
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

        if (animationComponent.GetPlaybackType() == AnimationComponent::PlaybackType::BlendTree1D) {
            ProcessBlendTree1D(animationComponent, *modelComponent, *modelResource, deltaTime);
        }
        else if (animationComponent.IsTransitioning()) {
            ProcessTransition(animationComponent, *modelComponent, *modelResource, deltaTime);
        }
        else {
            ProcessSingleClip(animationComponent, *modelComponent, *modelResource, deltaTime);
        }
    }
}

/// @brief 単一のアニメーションクリップを再生する
void AnimationProcessor::ProcessSingleClip(
    AnimationComponent& animationComponent,
    ModelComponent& modelComponent,
    ModelResource& modelResource,
    float deltaTime)
{
    AnimationState& state = animationComponent.GetAnimationState();
    if (state.clipName == AnimationComponent::CLIP_NONE) return;

    const AnimationClip* clip = ResolveAnimationClip(state, modelResource);
    if (!clip) return;
    const float animationTime = AdvanceAnimationState(state, *clip, deltaTime);

    SkeletonPose pose = modelResource.defaultPose;
    ApplyLocalPose(pose, SampleLocalPose(*clip, pose, animationTime));
    BuildSkeletonMatrices(pose, modelResource);
    modelComponent.SetSkeletonPose(pose);
}

/// @brief 遷移元と遷移先のアニメーション姿勢を時間に応じて補間する
void AnimationProcessor::ProcessTransition(
    AnimationComponent& animationComponent,
    ModelComponent& modelComponent,
    ModelResource& modelResource,
    float deltaTime)
{
    AnimationTransition& transition = animationComponent.GetTransitionState();
    AnimationState& destinationState = animationComponent.GetAnimationState();

    const AnimationClip* sourceClip = ResolveAnimationClip(
        transition.sourceState,
        modelResource);
    const AnimationClip* destinationClip = ResolveAnimationClip(
        destinationState,
        modelResource);

    if (!sourceClip || !destinationClip || transition.duration <= 0.0f) {
        animationComponent.CompleteTransition();
        ProcessSingleClip(animationComponent, modelComponent, modelResource, deltaTime);
        return;
    }

    const float sourceTime = AdvanceAnimationState(
        transition.sourceState,
        *sourceClip,
        deltaTime);
    const float destinationTime = AdvanceAnimationState(
        destinationState,
        *destinationClip,
        deltaTime);

    transition.timer += deltaTime;
    float weight = MiMath::Clamp(
        transition.timer / transition.duration,
        0.0f,
        1.0f);
    // 遷移の開始・終了付近を滑らかにする
    weight = weight * weight * (3.0f - 2.0f * weight);

    SkeletonPose pose = modelResource.defaultPose;
    const LocalPose sourcePose = SampleLocalPose(*sourceClip, pose, sourceTime);
    const LocalPose destinationPose = SampleLocalPose(
        *destinationClip,
        pose,
        destinationTime);
    ApplyLocalPose(pose, BlendLocalPoses(sourcePose, destinationPose, weight));
    BuildSkeletonMatrices(pose, modelResource);
    modelComponent.SetSkeletonPose(pose);

    if (transition.timer >= transition.duration) {
        animationComponent.CompleteTransition();
    }
}

/// @brief 1D BlendTreeを再生する
void AnimationProcessor::ProcessBlendTree1D(
    AnimationComponent& animationComponent,
    ModelComponent& modelComponent,
    ModelResource& modelResource,
    float deltaTime)
{
    AnimationBlendTree1DState& state = animationComponent.GetBlendTree1DState();

    std::vector<const AnimationBlendTree1DNode*> validNodes;
    validNodes.reserve(state.nodes.size());
    for (const AnimationBlendTree1DNode& node : state.nodes) {
        if (node.clipIndex < 0 || node.clipIndex >= static_cast<int>(modelResource.animationClips.size())) continue;
        if (modelResource.animationClips[node.clipIndex].duration <= 0.0f) continue;
        validNodes.push_back(&node);
    }
    if (validNodes.empty()) return;

    size_t lowerIndex = 0;
    size_t upperIndex = 0;
    float blendWeight = 0.0f;
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

    SkeletonPose pose = modelResource.defaultPose;
    const LocalPose lowerPose = SampleLocalPose(
        lowerClip, pose, normalizedTime * lowerClip.duration);

    LocalPose resultPose = lowerPose;
    if (lowerIndex != upperIndex) {
        const LocalPose upperPose = SampleLocalPose(
            upperClip, pose, normalizedTime * upperClip.duration);
        resultPose = BlendLocalPoses(lowerPose, upperPose, blendWeight);
    }

    ApplyLocalPose(pose, resultPose);
    BuildSkeletonMatrices(pose, modelResource);
    modelComponent.SetSkeletonPose(pose);
}

/// @brief AnimationStateが参照するクリップを取得する
const AnimationClip* AnimationProcessor::ResolveAnimationClip(
    AnimationState& state,
    ModelResource& modelResource)
{
    if (state.clipName == AnimationComponent::CLIP_NONE) return nullptr;

    if (state.clipIndex >= 0 &&
        state.clipIndex < static_cast<int>(modelResource.animationClips.size())) {
        const AnimationClip& clip = modelResource.animationClips[state.clipIndex];
        return clip.duration > 0.0f ? &clip : nullptr;
    }

    const auto it = std::find_if(
        modelResource.animationClips.begin(),
        modelResource.animationClips.end(),
        [&state](const AnimationClip& clip) {
            return clip.name == state.clipName;
        });
    if (it == modelResource.animationClips.end() || it->duration <= 0.0f) {
        state.clipName = AnimationComponent::CLIP_NONE;
        state.clipIndex = -1;
        return nullptr;
    }

    state.clipIndex = static_cast<int>(
        std::distance(modelResource.animationClips.begin(), it));
    return &*it;
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
    const SkeletonPose& basePose,
    float animationTime)
{
    LocalPose result{
        basePose.defaultPositions,
        basePose.defaultRotations,
        basePose.defaultScales,
    };

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
