//===================================================
// File  ：_/Enemy/enemy_animation_controller.cpp
// Date  ：2026/09/10
// Author：Miu Kitamura
//===================================================
#include "enemy_animation_controller.h"

#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"
#include "Game/ActorBehavior/Enemy/E20_Condition/enemy_condition_machine.h"
#include "Engine/Component/animation_component.h"
#include "Engine/Component/model_component.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Core/game_object.h"
#include "Engine/Graphics/model_repository.h"
#include "Engine/engine_service_locator.h"
#include <cmath>
#include <filesystem>

namespace {
    int FindOrLoadClip(ModelResource* model, const std::filesystem::path& path)
    {
        // 複数の敵が共有するModelResourceへ、同じクリップを重複登録しない。
        for (size_t i = 0; i < model->animationClips.size(); ++i) {
            if (model->animationClips[i].filePath == path) return static_cast<int>(i);
        }
        auto* repository = Engine::ModelRepository();
        return repository ? repository->LoadAnimation(model, path) : -1;
    }
}

void EnemyAnimationController::Initialize(EnemyContext& context)
{
    Finalize();
    auto* owner = context.transform ? context.transform->GetOwner() : nullptr;
    if (!owner) return;

    m_animationComponent = owner->GetComponent<AnimationComponent>();
    auto* modelComponent = owner->GetComponent<ModelComponent>();
    auto* model = modelComponent ? modelComponent->GetModelResource() : nullptr;
    if (!m_animationComponent || !model) return;

    m_idleClipIndex = FindOrLoadClip(model, "asset/Model/enemy_a_idle.anim.fbx");
    m_walkClipIndex = FindOrLoadClip(model, "asset/Model/enemy_a_walk.anim.fbx");
    PlayAnimation(Animation::Idle);
}

void EnemyAnimationController::Update(EnemyContext& context)
{
    if (!m_animationComponent) return;

    if (context.conditionMachine) {
        const auto condition = context.conditionMachine->GetCurrentCondition();
        if (condition == EnemyCondition::Stun || condition == EnemyCondition::Dead) {
            PlayAnimation(Animation::Idle);
            return;
        }
    }

    const auto velocity = context.runtimeState.controlVelocity;
    const float speed = std::hypot(velocity.x, velocity.z);
    Animation next = m_currentAnimation;
    if (m_currentAnimation == Animation::Walk) {
        if (speed <= m_settings.walkStopSpeed) next = Animation::Idle;
    }
    else if (speed >= m_settings.walkStartSpeed) {
        next = Animation::Walk;
    }
    PlayAnimation(next);
}

void EnemyAnimationController::PlayAnimation(Animation animation)
{
    if (!m_animationComponent) return;
    // Walkが読み込めなかった場合はIdleへフォールバックする。
    if (animation == Animation::Walk && m_walkClipIndex < 0) animation = Animation::Idle;
    const int clipIndex = animation == Animation::Walk ? m_walkClipIndex : m_idleClipIndex;
    if (clipIndex < 0 || (m_hasCurrentAnimation && animation == m_currentAnimation)) return;

    const float playbackSpeed = animation == Animation::Walk
        ? m_settings.walkPlaybackSpeed : m_settings.idlePlaybackSpeed;
    m_animationComponent->PlayAnimation(clipIndex, playbackSpeed, true, false,
        m_hasCurrentAnimation ? m_settings.transitionTime : 0.0f);
    m_currentAnimation = animation;
    m_hasCurrentAnimation = true;
}

void EnemyAnimationController::Finalize()
{
    m_animationComponent = nullptr;
    m_idleClipIndex = -1;
    m_walkClipIndex = -1;
    m_currentAnimation = Animation::Idle;
    m_hasCurrentAnimation = false;
}
