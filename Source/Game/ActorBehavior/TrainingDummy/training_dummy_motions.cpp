//===================================================
// File  ：_/TrainingDummy/training_dummy_motions.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "training_dummy_motions.h"

#include <algorithm>
#include <cmath>

#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"

#include "Engine/Component/transform_component.h"
#include "Engine/Component/model_component.h"

#include "Engine/engine_service_locator.h"

#include "Utility/mi_math.h"

namespace {
    // FIX: 将来的にはUtilityにしたい
    int FindClipIndex(const ModelComponent* modelComponent, const std::filesystem::path& fileName) {
        if (!modelComponent || !modelComponent->GetModelResource()) return -1;

        const auto& clips = modelComponent->GetModelResource()->animationClips;
        const auto it = std::find_if(clips.begin(), clips.end(), [&fileName](const AnimationClip& clip) {
            return clip.filePath.filename() == fileName;
            });
        return it == clips.end() ? -1 : static_cast<int>(std::distance(clips.begin(), it));
    }
}

void TrainingDummyMotions::Initialize(GameObject* owner)
{
    m_transform = owner->GetComponent<TransformComponent>();
    m_modelComponent = owner->GetComponent<ModelComponent>();
    m_animationComponent = owner->GetComponent<AnimationComponent>();
    ModelResource* modelResource = m_modelComponent ? m_modelComponent->GetModelResource() : nullptr;

    if (modelResource) {
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/kakashi_idle.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/kakashi_break_forward.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/kakashi_break_back.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/kakashi_break_right.anim.fbx");
        Engine::ModelRepository()->LoadAnimation(modelResource, "asset/Model/kakashi_break_left.anim.fbx");
    }

    m_knockbackBlendTreeNodes = {
        { FindClipIndex(m_modelComponent, "kakashi_idle.anim.fbx"), { 0.0f, 0.0f } },
        { FindClipIndex(m_modelComponent, "kakashi_break_forward.anim.fbx"), { 0.0f, 1.0f } },
        { FindClipIndex(m_modelComponent, "kakashi_break_back.anim.fbx"), { 0.0f, -1.0f } },
        { FindClipIndex(m_modelComponent, "kakashi_break_right.anim.fbx"), { 1.0f, 0.0f } },
        { FindClipIndex(m_modelComponent, "kakashi_break_left.anim.fbx"), { -1.0f, 0.0f } },
    };
    m_animationComponent->PlayBlendTree2D(m_knockbackBlendTreeNodes, m_currentKnockbackParameter);
}

void TrainingDummyMotions::Update(float deltaTime)
{
    if (!m_animationComponent || deltaTime <= 0.0f) return;

    // 小刻みに積分して、フレーム時間が長い場合もばねを安定させる
    constexpr float maxStep = 1.0f / 240.0f;
    float remainingTime = deltaTime;
    while (remainingTime > 0.0f) {
        const bool isKnockback = m_knockbackMotionTimer > 0.0f;
        float stepTime = (std::min)(remainingTime, maxStep);

        if (isKnockback) {
            stepTime = (std::min)(stepTime, m_knockbackMotionTimer);
        }
        const DirectX::XMFLOAT2 target = isKnockback
            ? m_targetKnockbackParameter
            : DirectX::XMFLOAT2{ 0.0f, 0.0f };

        const float stiffness = m_springAngularFrequency * m_springAngularFrequency;
        const float damping = 2.0f * m_springDampingRatio * m_springAngularFrequency;
        // 加速度を算出して適用してるだけ
        const auto stepAxis = [&](float& position, float& velocity, float targetValue) {
            const float acceleration = stiffness * (targetValue - position) - damping * velocity;
            velocity += acceleration * stepTime;
            position += velocity * stepTime;
        };
        stepAxis(m_currentKnockbackParameter.x, m_knockbackParameterVelocity.x, target.x);
        stepAxis(m_currentKnockbackParameter.y, m_knockbackParameterVelocity.y, target.y);

        m_knockbackMotionTimer = (std::max)(0.0f, m_knockbackMotionTimer - stepTime);
        remainingTime -= stepTime;
    }

    // 位置と速度の両方が十分小さくなったら、静止姿勢へ収束
    constexpr float positionEpsilon = 0.001f;
    constexpr float velocityEpsilon = 0.001f;
    if (m_knockbackMotionTimer <= 0.0f &&
        std::abs(m_currentKnockbackParameter.x) < positionEpsilon &&
        std::abs(m_currentKnockbackParameter.y) < positionEpsilon &&
        std::abs(m_knockbackParameterVelocity.x) < velocityEpsilon &&
        std::abs(m_knockbackParameterVelocity.y) < velocityEpsilon) {
        m_currentKnockbackParameter = {};
        m_knockbackParameterVelocity = {};
    }

    m_animationComponent->SetBlendTree2DParameter(m_currentKnockbackParameter);
}

void TrainingDummyMotions::PlayKnockbackMotion(const DirectX::XMFLOAT3& direction, float powerRate, float duration)
{
    // ローカルな攻撃方向に変換
    DirectX::XMFLOAT3 localAttackDirection = MiMath::HorizontalNormalize(direction);
    localAttackDirection = MiMath::RotateVector(m_transform->GetRotation(), localAttackDirection);

    // 2Dパラメータに変換
    DirectX::XMFLOAT2 knockbackParameter = { localAttackDirection.x * powerRate, localAttackDirection.z * powerRate };
    m_targetKnockbackParameter = knockbackParameter;

    // 再被弾でも現在の姿勢と速度は維持する。
    m_knockbackMotionTimer = duration;
}
