#include "enemy_motions.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

#include "Engine/Core/game_object.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/engine_service_locator.h"
#include "Utility/mi_math.h"

namespace {
    int FindClipIndex(const ModelComponent* modelComponent, const std::filesystem::path& fileName)
    {
        if (!modelComponent || !modelComponent->GetModelResource()) return -1;
        const auto& clips = modelComponent->GetModelResource()->animationClips;
        const auto it = std::find_if(clips.begin(), clips.end(), [&fileName](const AnimationClip& clip) {
            return clip.filePath.filename() == fileName;
        });
        return it == clips.end() ? -1 : static_cast<int>(std::distance(clips.begin(), it));
    }
}

void EnemyMotions::Initialize(GameObject* owner)
{
    if (!owner) return;

    m_transform = owner->GetComponent<TransformComponent>();
    m_animationComponent = owner->GetComponent<AnimationComponent>();
    ModelComponent* modelComponent = owner->GetComponent<ModelComponent>();
    ModelResource* modelResource = modelComponent ? modelComponent->GetModelResource() : nullptr;
    if (!m_animationComponent || !modelResource) return;

    Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/enemy_a_break_forward.anim.fbx");
    Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/enemy_a_break_back.anim.fbx");
    Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/enemy_a_break_right.anim.fbx");
    Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/enemy_a_break_left.anim.fbx");

    m_hitBlendTreeNodes = {
        { FindClipIndex(modelComponent, "enemy_a_break_forward.anim.fbx"), { 0.0f, 1.0f } },
        { FindClipIndex(modelComponent, "enemy_a_break_back.anim.fbx"), { 0.0f, -1.0f } },
        { FindClipIndex(modelComponent, "enemy_a_break_right.anim.fbx"), { 1.0f, 0.0f } },
        { FindClipIndex(modelComponent, "enemy_a_break_left.anim.fbx"), { -1.0f, 0.0f } },
    };

    // Bodyとその子ボーンだけへ被弾姿勢をOverride合成する。
    const AnimationBoneMask bodyMask = AnimationComponent::CreateBoneMask(*modelResource, "Body");
    m_hitLayerIndex = m_animationComponent->AddAnimationLayer(bodyMask);
    m_animationComponent->StopAnimationLayer(m_hitLayerIndex);
}

void EnemyMotions::Update(float deltaTime)
{
    if (!m_animationComponent || !m_isPlaying) return;

    if (m_holdingHitPose) {
        m_hitStopTask.Update(FPS_GetUnscaledDeltaTime());
        if (m_hitStopTask.IsRunning()) return;
        CancelHitStop();
    }
    if (deltaTime <= 0.0f) return;

    constexpr float maxStep = 1.0f / 240.0f;
    float remainingTime = deltaTime;
    while (remainingTime > 0.0f) {
        const bool receivingHit = m_hitMotionTimer > 0.0f;
        float stepTime = (std::min)(remainingTime, maxStep);
        if (receivingHit) stepTime = (std::min)(stepTime, m_hitMotionTimer);

        const DirectX::XMFLOAT2 target = receivingHit
            ? m_targetHitParameter
            : DirectX::XMFLOAT2{};
        const float stiffness = m_springAngularFrequency * m_springAngularFrequency;
        const float damping = 2.0f * m_springDampingRatio * m_springAngularFrequency;
        const auto stepAxis = [&](float& position, float& velocity, float targetValue) {
            const float acceleration = stiffness * (targetValue - position) - damping * velocity;
            velocity += acceleration * stepTime;
            position += velocity * stepTime;
        };
        stepAxis(m_currentHitParameter.x, m_hitParameterVelocity.x, target.x);
        stepAxis(m_currentHitParameter.y, m_hitParameterVelocity.y, target.y);

        m_hitMotionTimer = (std::max)(0.0f, m_hitMotionTimer - stepTime);
        remainingTime -= stepTime;
    }

    constexpr float epsilon = 0.001f;
    if (m_hitMotionTimer <= 0.0f
        && std::abs(m_currentHitParameter.x) < epsilon
        && std::abs(m_currentHitParameter.y) < epsilon
        && std::abs(m_hitParameterVelocity.x) < epsilon
        && std::abs(m_hitParameterVelocity.y) < epsilon) {
        Stop();
        return;
    }

    ApplyLayerParameter(false);
}

void EnemyMotions::PlayKnockbackMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration)
{
    if (!m_transform || !m_animationComponent || m_hitLayerIndex == InvalidLayerIndex) return;

    DirectX::XMFLOAT3 localDirection = MiMath::HorizontalNormalize(direction);
    localDirection = MiMath::RotateVector(m_transform->GetRotation(), localDirection);
    m_targetHitParameter = { localDirection.x * powerRate, localDirection.z * powerRate };
    m_hitMotionTimer = duration;
    m_isPlaying = true;
    ApplyLayerParameter(false);
}

void EnemyMotions::PlaySlashHitMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration)
{
    if (!m_animationComponent) return;

    PlayKnockbackMotion(direction, powerRate, 0.0f);
    m_currentHitParameter = m_targetHitParameter;
    m_hitParameterVelocity = {};
    ApplyLayerParameter(true);

    if (duration <= 0.0f) return;
    m_holdingHitPose = true;
    m_animationComponent->SetPaused(true);
    m_hitStopTask.RequestHold((std::max)(duration, m_hitStopTask.GetRemainingTime()));
}

void EnemyMotions::CancelHitStop()
{
    if (m_holdingHitPose && m_animationComponent) m_animationComponent->SetPaused(false);
    m_holdingHitPose = false;
    m_hitStopTask.Reset();
}

void EnemyMotions::Stop()
{
    CancelHitStop();
    if (m_animationComponent && m_hitLayerIndex != InvalidLayerIndex) {
        m_animationComponent->StopAnimationLayer(m_hitLayerIndex);
    }
    m_currentHitParameter = {};
    m_targetHitParameter = {};
    m_hitParameterVelocity = {};
    m_hitMotionTimer = 0.0f;
    m_isPlaying = false;
}

void EnemyMotions::ApplyLayerParameter(bool restart)
{
    if (!m_animationComponent || m_hitLayerIndex == InvalidLayerIndex || m_hitBlendTreeNodes.empty()) return;
    m_animationComponent->PlayLayerBlendTree2D(
        m_hitLayerIndex,
        m_hitBlendTreeNodes,
        m_currentHitParameter,
        1.0f,
        true,
        restart,
        0.0f);
}
