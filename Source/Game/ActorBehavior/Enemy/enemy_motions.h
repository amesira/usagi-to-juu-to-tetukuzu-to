#pragma once

#include <DirectXMath.h>
#include <limits>
#include <vector>

#include "Engine/Component/animation_component.h"
#include "Game/ActorBehavior/Base/ReactionEffects/hit_stop_sequence_task.h"

class EnemyMotions {
private:
    static constexpr size_t InvalidLayerIndex = (std::numeric_limits<size_t>::max)();

    class TransformComponent* m_transform = nullptr;
    AnimationComponent* m_animationComponent = nullptr;
    size_t m_hitLayerIndex = InvalidLayerIndex;
    std::vector<AnimationBlendTree2DNode> m_hitBlendTreeNodes;

    float m_springAngularFrequency = 24.0f;
    float m_springDampingRatio = 0.25f;
    DirectX::XMFLOAT2 m_currentHitParameter = {};
    DirectX::XMFLOAT2 m_targetHitParameter = {};
    DirectX::XMFLOAT2 m_hitParameterVelocity = {};
    float m_hitMotionTimer = 0.0f;

    HitStopTask m_hitStopTask;
    bool m_holdingHitPose = false;
    bool m_isPlaying = false;

public:
    void Initialize(class GameObject* owner);
    void Update(float deltaTime);
    void PlaySlashHitMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration);
    void PlayKnockbackMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration);
    void CancelHitStop();
    void Stop();

    bool IsPlaying() const { return m_isPlaying; }

private:
    void ApplyLayerParameter(bool restart);
};
