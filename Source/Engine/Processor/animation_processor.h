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
    void ProcessSingleClip(
        class AnimationComponent& animationComponent,
        class ModelComponent& modelComponent,
        ModelResource& modelResource,
        float deltaTime);

    void ProcessTransition(
        class AnimationComponent& animationComponent,
        class ModelComponent& modelComponent,
        ModelResource& modelResource,
        float deltaTime);

    void ProcessBlendTree1D(
        class AnimationComponent& animationComponent,
        class ModelComponent& modelComponent,
        ModelResource& modelResource,
        float deltaTime);

    void ProcessBlendTree2D(
        class AnimationComponent& animationComponent,
        class ModelComponent& modelComponent,
        ModelResource& modelResource,
        float deltaTime);

    void ProcessAnimationLayers(
        class AnimationComponent& animationComponent,
        class ModelComponent& modelComponent,
        ModelResource& modelResource,
        float deltaTime);

    LocalPose SampleLocalPose(
        const AnimationClip& clip,
        const SkeletonPose& basePose,
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
    LocalPose ExtractLocalPose(const SkeletonPose& pose);
    void ApplyLocalPose(SkeletonPose& pose, const LocalPose& localPose);
    void BuildSkeletonMatrices(SkeletonPose& pose, const ModelResource& modelResource);

    // キーフレームの線形補間
    XMFLOAT4 SamplingKeyframes(
        const std::vector<AnimationClip::Keyframe>& keyframes,
        float time,
        bool useQuaternionSlerp = false);

};

#endif // ANIMATION_PROCESSOR_H
