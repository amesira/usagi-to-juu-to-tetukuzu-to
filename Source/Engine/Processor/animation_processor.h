//---------------------------------------------------
// animation_processor.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/06
//---------------------------------------------------
#ifndef ANIMATION_PROCESSOR_H
#define ANIMATION_PROCESSOR_H
#include "Engine/Core/processor.h"
#include "Engine/Graphics/model_resource.h"

class AnimationProcessor : public Processor {
private:

    struct LocalPose {
        std::vector<XMFLOAT4> positions;
        std::vector<XMFLOAT4> rotations;
        std::vector<XMFLOAT4> scales;
    };

public:
    void Initialize() override;
    void Finalize() override;
    void Process(IScene* pScene) override;

private:
    bool EvaluatePlaybackState(
        LocalPose& outPose,
        struct AnimationPlaybackState& playbackState,
        ModelResource& modelResource,
        const LocalPose& fallbackPose,
        float deltaTime);

    bool EvaluateSingleClip(
        LocalPose& outPose,
        struct AnimationState& state,
        ModelResource& modelResource,
        const LocalPose& fallbackPose,
        float deltaTime);

    bool EvaluateTransition(
        LocalPose& outPose,
        struct AnimationPlaybackState& playbackState,
        ModelResource& modelResource,
        const LocalPose& fallbackPose,
        float deltaTime);

    bool EvaluateBlendTree1D(
        LocalPose& outPose,
        struct AnimationBlendTree1DState& state,
        ModelResource& modelResource,
        const LocalPose& fallbackPose,
        float deltaTime);

    bool EvaluateBlendTree2D(
        LocalPose& outPose,
        struct AnimationBlendTree2DState& state,
        ModelResource& modelResource,
        const LocalPose& fallbackPose,
        float deltaTime);

    LocalPose SampleLocalPose(
        const AnimationClip& clip,
        const LocalPose& fallbackPose,
        float animationTime);

    const AnimationClip* ResolveAnimationClip(
        struct AnimationState& state,
        ModelResource& modelResource);
    float AdvanceAnimationState(
        struct AnimationState& state,
        const AnimationClip& clip,
        float deltaTime);

    LocalPose BlendLocalPoses(const LocalPose& lhs, const LocalPose& rhs, float weight);
    LocalPose BlendLocalPosesMasked(
        const LocalPose& basePose,
        const LocalPose& layerPose,
        const struct AnimationBoneMask& mask,
        float layerWeight);
    LocalPose MakeDefaultLocalPose(const SkeletonPose& pose);
    void ApplyLocalPose(SkeletonPose& pose, const LocalPose& localPose);
    void BuildSkeletonMatrices(SkeletonPose& pose, const ModelResource& modelResource);

    // キーフレームの線形補間
    XMFLOAT4 SamplingKeyframes(
        const std::vector<AnimationClip::Keyframe>& keyframes,
        float time,
        bool useQuaternionSlerp = false);

};

#endif // ANIMATION_PROCESSOR_H
