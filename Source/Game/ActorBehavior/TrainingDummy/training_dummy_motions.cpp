//===================================================
// File  ：_/TrainingDummy/training_dummy_motions.cpp
// Date  ：2026/09/07
// Author：Miu Kitamura
//===================================================
#include "training_dummy_motions.h"

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
    if (!m_animationComponent) return;
    m_knockbackMotionTimer -= deltaTime;

    // ノックバック中（のけぞっている）
    if (m_knockbackMotionTimer > 0.0f) {
        m_currentKnockbackParameter.x = MiMath::SmoothDamp(
            m_currentKnockbackParameter.x, 
            m_targetKnockbackParameter.x, 
            m_knockbackParameterVelocity.x, 
            m_motionSmoothTime, 
            deltaTime);
        m_currentKnockbackParameter.y = MiMath::SmoothDamp(
            m_currentKnockbackParameter.y, 
            m_targetKnockbackParameter.y, 
            m_knockbackParameterVelocity.y, 
            m_motionSmoothTime, 
            deltaTime);
    }
    // ノックバック終了後、元の姿勢に戻る
    else {
        m_currentKnockbackParameter.x = MiMath::SmoothDamp(
            m_currentKnockbackParameter.x, 
            0.0f, 
            m_knockbackParameterVelocity.x, 
            m_endMotionSmoothTime, 
            deltaTime);
        m_currentKnockbackParameter.y = MiMath::SmoothDamp(
            m_currentKnockbackParameter.y, 
            0.0f, 
            m_knockbackParameterVelocity.y, 
            m_endMotionSmoothTime, 
            deltaTime);
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

    m_knockbackMotionTimer = duration;
}
